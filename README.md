# DevicePilotHMI

DevicePilotHMI is a Qt Quick / QML desktop HMI application for a simulated industrial machine. It demonstrates a complete HMI workflow including real-time telemetry visualization, alarm lifecycle management, event logging, and settings management.

## What is This Project?

This project is a desktop HMI (Human-Machine Interface) for a simulated industrial machine that demonstrates:

- **Real-time telemetry visualization** with temperature, pressure, and speed metrics
- **Trend charts** with warning/fault threshold overlays
- **Alarm lifecycle management** with states for Normal, WarningActive, FaultLatched, ResetRequested, and RecoveryNotice
- **Event logging** with filtering, acknowledgment, and persisted column visibility
- **Settings management** with threshold presets, draft editing, validation, and JSON persistence
- **Runtime-aware policy** controlling what settings can be changed at different machine states

## Why Does This Project Exist?

This project was created to demonstrate a complete, structured Qt/QML application that goes beyond simple demos:

- It keeps application state in C++ while using QML for presentation
- It separates backend interface from simulator implementation
- It implements proper alarm lifecycle states instead of simple boolean alarms
- It supports session-based runtime logging for engineering diagnostics
- It includes automated unit tests for core business logic

The project serves as:
- A reference architecture for Qt/QML applications
- A portfolio piece demonstrating professional HMI development
- A starting point for real-device integration projects

## Quick Start

### Windows

```powershell
# Install Qt 6.11 with MinGW toolchain
# Set up environment
.\scripts\setup_windows_qt_env.ps1 -QtRoot 'C:\Qt' -Scope User

# Build
.\scripts\build.ps1 -Preset windows-mingw-debug

# Run
.\build\windows-mingw-debug\bin\DevicePilotHMI.exe
```

### Linux

```bash
# Install Qt 6.11 and build tools
./scripts/build.sh --preset linux-ninja-debug

# Run
./build/linux-ninja-debug/bin/DevicePilotHMI
```

## Application Flow

1. Start the machine from the dashboard
2. Watch telemetry rise in the running state
3. Observe warning, fault, reset, and recovery transitions when thresholds are crossed
4. Open the event log to inspect runtime and config activity
5. Open settings to apply a threshold preset or edit thresholds directly
6. Apply or revert the draft and confirm persistence across restarts

## Tech Stack

- C++20
- Qt 6 Quick / QML
- CMake 3.25+

## Development with AI Assistance

This project was developed with assistance from AI tools, including:

- **Unit test cases**: Many unit tests in the `tests/` directory were generated with AI assistance
- **QML UI pages**: The UI pages (Dashboard, Event Log, Settings) were developed with AI help
- **README documentation**: This README was written with AI assistance

AI assistance helped accelerate development while maintaining code quality through automated testing and code review workflows.

## Build

### Requirements

- CMake 3.25 or newer
- Qt 6.11 or newer with Qt Quick, QML, and Test
- C++20 capable compiler

### Windows Build

```powershell
# Set up environment (run once)
.\scripts\setup_windows_qt_env.ps1 -QtRoot 'C:\Qt' -Scope User

# Build with MinGW
.\scripts\build.ps1 -Preset windows-mingw-debug

# Or build with MSVC
.\scripts\build.ps1 -Preset windows-msvc-debug

# Create distributable package
.\scripts\package_windows.ps1 -Preset windows-mingw-release
```

### Linux Build

```bash
# Build
./scripts/build.sh --preset linux-ninja-debug

# Or with custom Qt
./scripts/build.sh --preset linux-ninja-debug --qt-root /path/to/Qt/6.11.0/gcc_64
```

## Automated Tests

The project includes unit tests for:

- Settings validation, JSON codec, file store, and manager
- Machine runtime start, stop, fault, and reset behavior
- Alarm lifecycle transitions and threshold evaluation
- Backend simulation scenarios

Run tests with:

```powershell
# Windows
ctest --preset windows-mingw-debug

# Linux
ctest --preset linux-ninja-debug
```

## Project Structure

```
src/
├── main.cpp              # Application entry point
├── alarm/               # Alarm lifecycle management
├── backend/             # Machine backend interface
├── log/                 # Event logging
├── runtime/             # Machine runtime state
└── settings/           # Settings management
qml/
├── pages/              # Dashboard, Log, Settings pages
└── components/         # Reusable UI components
tests/                  # Unit tests
scripts/               # Build and packaging scripts
.github/
└── workflows/         # CI workflows
```

## License

MIT License. See [LICENSE](LICENSE).

## Application Overview

The app is split into three top-level pages:

- `Dashboard`
  - shows temperature, pressure, speed, machine status, and alarm state
  - renders trend charts with warning/fault threshold overlays and history markers
  - keeps a primary alarm focus through `activeMetric` while still showing live per-metric warning/fault state on the cards
  - allows `Start`, `Stop`, and `Reset Fault`
- `Event Log`
  - shows runtime, config, warning, and fault events
  - supports filtering, acknowledgment, and column visibility preferences
  - remains an in-memory operator-facing event stream exposed through `LogModel`
- `Settings`
  - edits warning/fault thresholds and update interval
  - supports `Conservative`, `Balanced`, and `Aggressive` threshold presets
  - validates input before applying
  - persists committed settings and log view preferences to JSON

Separate from the `Event Log` page, the application also writes session log files under the platform local app-data directory for engineering diagnostics.

## Runtime Behavior

The machine starts in `Idle`.

- `Start`
  - transitions to `Starting`
  - after 5 seconds enters `Running`
- `Running`
  - temperature, pressure, and speed increase over time
  - alarm rules are evaluated continuously
- `Stop`
  - transitions to `Stopping`
  - after 1.2 seconds returns to `Idle`
- `Fault`
  - entered when temperature or pressure reaches a fault threshold
  - runtime timers stop
  - `Reset Fault` restores the machine to idle values

## Alarm Model

`AlarmManager` owns the alarm state exposed to the dashboard.

- `Normal`
  - banner text: `System normal`
- `WarningActive`
  - raised when warning thresholds are exceeded while the machine is still running
  - exposes the active metric, threshold detail, trend direction, and operator hint
- `FaultLatched`
  - raised when fault thresholds are exceeded
  - forces the runtime into fault state and requests a backend safe shutdown
- `ResetRequested`
  - entered after the operator requests `Reset Fault` and before the backend reports `Idle`
- `RecoveryNotice`
  - shown briefly after fault reset completes and the machine returns to `Idle`

Current alarm evaluation is threshold-based and uses the committed settings snapshot. Threshold applies performed while the machine is running trigger immediate reevaluation against current telemetry, so newly crossed thresholds can raise warning or fault state without waiting for a later sample.

The dashboard banner is driven by structured alarm presentation fields:

- `headline`
- `detail`
- `operatorHint`
- `stateLabel`
- `lifecycleState`
- `activeMetric`

For a deeper description of the lifecycle intent, see [docs/alarm-lifecycle-spec.md](docs/alarm-lifecycle-spec.md).

## Settings Model

The settings flow is intentionally split into separate responsibilities:

- `SettingsManager`
  - owns the committed settings snapshot and persisted log page view preferences
  - loads and persists settings
  - emits change signals for threshold changes and update interval changes
- `SettingsDraft`
  - owns editable form state used by the settings page
  - tracks validation and dirty state
- `Settings::Presets`
  - defines the threshold preset values for `Conservative`, `Balanced`, and `Aggressive`
- `SettingsApplyService`
  - decides whether a draft can be applied in the current runtime state
- `SettingsSession`
  - is the QML-facing page session object
  - exposes the draft, preset names, pending change count, and apply-related UI state

Validation rules include:

- temperature thresholds must be within valid numeric ranges
- pressure thresholds must be within valid numeric ranges
- update interval must be between `100` and `5000` ms
- warning thresholds must stay lower than their matching fault thresholds

## Persistence

Settings are stored as JSON in the platform application config directory.

The persisted configuration currently contains:

- `schemaVersion`
- committed warning/fault thresholds for temperature and pressure
- committed update interval
- `logPage` view preferences for timestamp/source/level column visibility

On Windows, the file is typically written to a path like:

```text
C:\Users\<User>\AppData\Local\DevicePilotHMI\devicepilothmi_settings.json
```

Behavior on startup:

- if the file does not exist, defaults are used
- if the file exists but is malformed or invalid, defaults are restored
- missing or invalid persisted `logPage` preferences are repaired to defaults
- repaired settings are written back to disk

## Runtime Logging

In addition to the in-app `Event Log` page, the application writes engineering-oriented session log files during startup and runtime.

The current implementation uses Qt logging categories for:

- `devicepilothmi.bootstrap`
- `devicepilothmi.runtime`
- `devicepilothmi.alarm`
- `devicepilothmi.settings`
- `devicepilothmi.persistence`
- `devicepilothmi.backend`
- `devicepilothmi.ui`

The session logger currently records:

- application startup, shutdown, and QML load success/failure
- settings initialization, apply success/rejection, validation failures, and persisted value changes
- settings file load success, repair flows, and persistence failures
- runtime start/stop/reset requests, machine state transitions, and fault reset completion
- alarm warning/fault/recovery lifecycle events
- backend scenario changes

To avoid unbounded growth, the logger currently keeps the most recent `10` session log files.

On Windows, session logs are typically written to a path like:

```text
C:\Users\<User>\AppData\Local\DevicePilotHMI\logs\devicepilothmi-<timestamp>-<pid>.log
```

The `Event Log` page and the session log files intentionally serve different purposes:

- the `Event Log` page is an in-memory business/event stream intended for UI inspection and filtering
- the session log files are durable engineering logs intended for troubleshooting and post-run inspection

## Current Limitations

- The runtime is still driven by a simulated backend; there is no real device backend yet.
- The alarm banner still presents one primary active metric at a time; the project does not yet implement a stacked or aggregated multi-alarm operator summary.
- The project still does not model a full industrial alarm system with shelving, suppression, historian integration, or alarm acknowledgment as part of the alarm lifecycle itself.
- The automated test suite currently focuses on core C++ modules and does not cover QML UI behavior end-to-end.
- The project is currently organized as a single application target rather than a split core library plus app shell.
