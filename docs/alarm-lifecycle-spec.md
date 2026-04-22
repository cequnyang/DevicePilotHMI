# Alarm Lifecycle Spec

This document defines a clearer fault lifecycle for DevicePilotHMI before code changes are made.

The goal is to make three things explicit in the UI and in the internal model:

1. the fault reason text;
2. the warning progression before fault;
3. the recovery notice after reset.

It is intentionally written against the current project structure:

- `MachineRuntime` owns machine state and reset handshake.
- `AlarmManager` owns derived alarm presentation state.
- QML consumes `AlarmManager` properties and `MachineRuntime` action availability.

## Current Problem

The current implementation compresses the whole chain into one active text:

- warning and fault both use `alarmText`;
- reset completion immediately restores `"System normal"`;
- the UI can show the current severity, but not the path the operator just went through.

This hides important operator-facing distinctions:

- warning acknowledged is not fault recovered;
- reset completed is not machine resumed;
- a cleared warning is not the same thing as a reset recovery notice.

## Target Lifecycle

Use the following alarm lifecycle states for presentation.

These are not the same as machine runtime states.

| Lifecycle state | Severity | Meaning | Typical banner label |
| --- | --- | --- | --- |
| `Normal` | normal | No active warning or fault. | `Normal` |
| `WarningActive` | warning | Threshold crossed, machine still running, operator should watch trend. | `Warning` |
| `FaultLatched` | fault | Fault condition triggered and machine entered fault-safe state. | `Fault` |
| `ResetRequested` | fault | Operator requested reset, backend has not yet reported idle recovery. | `Resetting` |
| `RecoveryNotice` | normal | Fault was cleared and machine returned to idle; short-lived success notice. | `Recovered` |

### Transition Table

| From | Trigger | To | Notes |
| --- | --- | --- | --- |
| `Normal` | warning threshold crossed | `WarningActive` | Enter only on first crossing, not every telemetry sample. |
| `WarningActive` | metric returns below warning threshold | `Normal` | This is a warning clear, not a recovery notice. |
| `WarningActive` | fault threshold crossed | `FaultLatched` | Fault entry should remember the warning context if one existed. |
| `Normal` | fault threshold crossed directly | `FaultLatched` | Possible if thresholds are tight or telemetry jumps. |
| `FaultLatched` | operator pressed reset | `ResetRequested` | Machine still not available for start. |
| `ResetRequested` | backend reports `Idle` after fault | `RecoveryNotice` | Show a short-lived recovery message. |
| `ResetRequested` | backend remains faulted or reset fails | `FaultLatched` | Optional future extension if backend can reject reset. |
| `RecoveryNotice` | timer expires or new start request occurs | `Normal` | Recovery notice is temporary. |

## Runtime vs Alarm Ownership

Keep the ownership split strict.

### `MachineRuntime`

Owns:

- runtime state: `Idle`, `Starting`, `Running`, `Stopping`, `Fault`;
- whether reset can be requested;
- whether reset is pending;
- transition completion from `Fault` to `Idle`.

Does not own:

- operator-facing explanation text;
- warning progression text;
- recovery banner text.

### `AlarmManager`

Owns:

- current alarm lifecycle state;
- current severity presentation;
- structured copy for banner and dashboard;
- short-term memory of the most recent fault and warning context.

This keeps machine control and operator messaging separated.

## Copy Contract

Do not drive the UI with one generic `alarmText`.

The UI should consume structured copy with stable meaning:

| Field | Purpose | Example |
| --- | --- | --- |
| `headline` | Short banner line. | `Over-temperature fault` |
| `detail` | Factual explanation with live or captured values. | `Temperature 96.2C exceeded fault limit 95C while Running.` |
| `operatorHint` | Next operator action. | `Inspect cooling path, then reset fault after values return to safe range.` |
| `stateLabel` | Small badge label. | `Fault`, `Warning`, `Recovered` |
| `activeMetric` | Metric to highlight in dashboard. | `temperature` |

### State-Specific Copy

#### `Normal`

- `headline`: `System normal`
- `detail`: `Temperature and pressure are within configured limits.`
- `operatorHint`: empty
- `stateLabel`: `Normal`

#### `WarningActive`

- `headline`: `Temperature warning` or `Pressure warning`
- `detail`: include:
  - current value;
  - warning threshold;
  - fault threshold;
  - remaining distance to fault threshold;
  - direction when available: `rising`, `falling`, or `steady`
- `operatorHint`: `Monitor trend and reduce load before fault threshold is reached.`
- `stateLabel`: `Warning`

Example warning detail:

`Temperature 82.4C exceeded warning limit 75C; 12.6C remaining to fault limit 95C; trend rising.`

#### `FaultLatched`

- `headline`: `Over-temperature fault` or `Over-pressure fault`
- `detail`: include:
  - captured trigger value;
  - fault threshold;
  - runtime state at trigger time, normally `Running`;
  - optional prior warning duration if available
- `operatorHint`: `Machine stopped in safe state. Inspect cause, then press Reset Fault.`
- `stateLabel`: `Fault`

Example fault detail:

`Temperature 96.2C exceeded fault limit 95C while Running. Warning was active before trip.`

#### `ResetRequested`

- `headline`: `Fault reset requested`
- `detail`: `Waiting for backend to return the machine from Fault to Idle.`
- `operatorHint`: `Do not start the machine until recovery is confirmed.`
- `stateLabel`: `Resetting`

#### `RecoveryNotice`

- `headline`: `Fault cleared`
- `detail`: `Machine returned to Idle after reset. Telemetry restored to safe idle values.`
- `operatorHint`: `System is ready for a new start sequence.`
- `stateLabel`: `Recovered`

This notice should not be replaced immediately by `System normal`.

Recommended lifetime: 3 to 5 seconds.

## Warning Progression Rules

The warning phase should explain progression, not only presence.

Recommended minimum data model for the warning presentation:

- warning metric key;
- current value;
- warning threshold;
- fault threshold;
- delta to fault threshold;
- trend direction from recent telemetry;
- warning start timestamp.

### Trend Rule

Trend can be derived cheaply from current and previous sample:

- current > previous: `rising`
- current < previous: `falling`
- otherwise: `steady`

Do not write a log entry for every trend update.

Trend is UI context, not an event.

### Warning Duration

When possible, remember when warning started.

This enables text such as:

`Warning active for 8s.`

That is much more useful than repeatedly showing the same threshold sentence.

## Event and Marker Mapping

Only edge transitions should emit log events and history markers.

Do not log every telemetry-driven text update.

| Lifecycle edge | Log level | Event type | Marker label |
| --- | --- | --- | --- |
| enter `WarningActive` | `WARNING` | `alarm.warning.entered` | `Warning` |
| leave `WarningActive` to `Normal` | `INFO` | `alarm.warning.cleared` | `Clear` |
| enter `FaultLatched` | `FAULT` | `alarm.fault.entered` | `Fault` |
| enter `ResetRequested` | `INFO` | `runtime.fault_reset.requested` | `Reset` |
| enter `RecoveryNotice` | `INFO` | `runtime.fault_reset.completed` | `Idle` |
| leave `RecoveryNotice` to `Normal` | optional | `alarm.recovery.notice.expired` | none |

### Important semantic boundaries

- Log acknowledgment means the operator saw an event.
- Recovery means the machine and alarm lifecycle returned to an acceptable state.
- These are different concerns and must remain separate.

## Proposed Minimal API Changes

The following API is the minimum needed to support the target lifecycle cleanly.

### `AlarmManager`

Keep existing properties temporarily for compatibility, but move QML gradually to these:

- `lifecycleState`
- `headline`
- `detail`
- `operatorHint`
- `stateLabel`
- `activeMetric`
- `warningActive`
- `faultActive`
- `recoveryActive`

Compatibility option:

- keep `alarmText` as an alias of `headline` for one transition step;
- then remove it after QML no longer depends on the old name.

### `MachineRuntime`

Existing state handling is mostly correct.

Minimal additions that may be useful:

- signal or property for `faultResetPending`;
- explicit notification when reset request starts, if QML or `AlarmManager` should display `ResetRequested` before the backend reports `Idle`.

## Minimal Implementation Order

1. Extend `AlarmManager` from one text field to structured presentation fields.
2. Add lifecycle memory for:
   - warning start time;
   - last trigger values;
   - last fault metric;
   - recovery notice timeout.
3. Distinguish these edges in `AlarmManager`:
   - warning entered;
   - warning cleared;
   - fault entered;
   - reset requested;
   - recovery notice entered;
   - recovery notice expired.
4. Update `StatusBanner.qml` to render:
   - `headline`;
   - optional shorter `detail` tooltip or expanded dashboard copy;
   - `stateLabel` including `Resetting` and `Recovered`.
5. Update dashboard detail panel to show:
   - warning progression text;
   - fault detail;
   - recovery notice text.
6. Add tests for:
   - warning-to-fault chain;
   - fault remains latched until reset;
   - reset enters recovery notice before normal;
   - warning clear does not use recovery wording.

## File Touch Plan

When implementation starts, the primary files should be:

- `src/alarm/alarm_manager.h`
- `src/alarm/alarm_manager.cpp`
- `src/runtime/machine_runtime.h`
- `src/runtime/machine_runtime.cpp`
- `qml/components/StatusBanner.qml`
- `qml/pages/DashboardPage.qml`
- `tests/alarm_manager_test.cpp`

## Non-Goals for the First Pass

Do not add these in the first implementation pass:

- backend-diagnosed root cause inference;
- reset failure retry flows unless the backend can explicitly report failure;
- historical alarm timeline UI beyond existing log and chart markers;
- multi-fault aggregation.

The first pass is only about making one warning/fault/reset chain understandable.
