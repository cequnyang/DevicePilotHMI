# DevicePilotHMI

DevicePilotHMI is a Qt Quick / QML desktop HMI demo for a simulated machine. It is designed as a small but structured project that keeps application state in C++ and uses QML mainly for presentation and interaction.

The project focuses on:

- a simulated runtime state machine
- a backend interface that keeps the application layer separate from the simulator implementation
- a threshold-based warning/fault lifecycle with reset and recovery presentation
- an event log with filtering, acknowledgment, and persisted column visibility
- editable, validated, preset-driven, and persisted settings
- a cleaner C++/QML integration boundary than a typical all-in-QML demo

## Highlights

- Dashboard page with live telemetry, trend charts, per-metric status cards, machine state context, and control actions
- Alarm banner with structured copy (`headline`, `detail`, `operatorHint`, `stateLabel`) instead of one generic text string
- Alarm lifecycle states for `Normal`, `WarningActive`, `FaultLatched`, `ResetRequested`, and `RecoveryNotice`
- Event log page with level filter, text search, "only unacknowledged" filtering, and persisted column visibility
- Settings page with threshold presets (`Conservative`, `Balanced`, `Aggressive`), draft editing, validation, apply/reload/factory defaults flow, and JSON persistence
- Runtime-aware settings policy:
  - all changes are allowed while the machine is idle
  - threshold changes are allowed while running
  - update interval changes are blocked while running
  - settings are blocked during starting, stopping, and fault states
- Applying threshold changes reevaluates the current telemetry immediately against the committed snapshot
- Settings are automatically loaded from disk, validated, and repaired to defaults if the file is missing or invalid

## Tech Stack

- C++20
- Qt 6 Quick
- Qt 6 QML
- CMake

The current `CMakeLists.txt` requires Qt 6.11 or newer.

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
- `Settings`
  - edits warning/fault thresholds and update interval
  - supports `Conservative`, `Balanced`, and `Aggressive` threshold presets
  - validates input before applying
  - persists committed settings and log view preferences to JSON

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

## Build

### Requirements

- CMake 3.25 or newer
- Qt 6.11 or newer with:
  - `Qt6::Quick`
  - `Qt6::Qml`
- a C++20-capable compiler
- on Windows:
  - MinGW 13.1 (or the Qt-provided `mingw1310_64` toolchain) for the MinGW presets
  - Visual Studio 2022 / Build Tools 2022 with the x64 C++ toolchain for the MSVC presets
- on Linux:
  - Ninja
  - a GCC or Clang toolchain compatible with Qt 6.11

### Fresh Windows Setup

For a new Windows machine, install the following first:

- Qt 6.11.0 from the Qt Online Installer
- `mingw_64` if you want to build with MinGW
- `msvc2022_64` if you want to build with MSVC
- `Tools/mingw1310_64` if you want to build with MinGW
- `Tools/Ninja`
- `Tools/CMake_64` or another CMake 3.25+ installation
- Visual Studio 2022 / Build Tools 2022 with the x64 C++ toolchain if you want to use the MSVC presets

A typical Qt installation layout is:

```text
C:\Qt\6.11.0\mingw_64
C:\Qt\6.11.0\msvc2022_64
C:\Qt\Tools\mingw1310_64
C:\Qt\Tools\Ninja
```

Initialize the preset environment once:

```powershell
.\scripts\setup_windows_qt_env.ps1 -QtRoot 'C:\Qt' -Scope User
```

Then use one of the checked-in presets.

MinGW:

```powershell
.\scripts\build.ps1 -Preset windows-mingw-debug
```

MSVC:

Open a Developer PowerShell for Visual Studio 2022 first, then run:

```powershell
.\scripts\build.ps1 -Preset windows-msvc-debug
```

The equivalent direct CMake command is:

```powershell
D:\Qt\Tools\CMake_64\bin\cmake.exe --build build/windows-msvc-debug --target ALL_BUILD --config Debug
```

If you need a clean rebuild after changing Qt, generators, or toolchains:

```powershell
.\scripts\build.ps1 -Preset windows-mingw-debug -Fresh
```

To create a redistributable Windows package:

```powershell
.\scripts\package_windows.ps1 -Preset windows-mingw-release
```

### Fresh Linux Setup

For a new Linux machine, install the following first:

- CMake 3.25 or newer
- Ninja
- GCC or Clang
- Qt 6.11 or newer with Quick, QML, and Test development packages

If you use a distro-provided Qt installation and CMake can already find it, build with:

```bash
./scripts/build.sh --preset linux-ninja-debug
```

If you use a Qt installation from the Qt Online Installer or another custom prefix, pass the Qt root explicitly:

```bash
./scripts/build.sh --preset linux-ninja-debug --qt-root /home/<user>/Qt/6.11.0/gcc_64
```

For a clean rebuild:

```bash
./scripts/build.sh --preset linux-ninja-debug --qt-root /home/<user>/Qt/6.11.0/gcc_64 --fresh
```

If you use Qt Creator on Linux, launch it from a shell that already exports the Qt location:

```bash
export QT_ROOT_DIR=/home/<user>/Qt/6.11.0/gcc_64
export CMAKE_PREFIX_PATH=/home/<user>/Qt/6.11.0/gcc_64
export PATH=/home/<user>/Qt/6.11.0/gcc_64/bin:$PATH
qtcreator /path/to/DevicePilotHMI
```

If configuration failed earlier, clear the failed CMake configuration or remove the corresponding `build/linux-*` directory before configuring again.

### Presets

The repository includes `CMakePresets.json` with matching Windows and Linux presets for Qt Creator and command-line builds:

- `windows-mingw-debug`
- `windows-mingw-release`
- `windows-msvc-debug`
- `windows-msvc-release`
- `linux-ninja-debug`
- `linux-ninja-release`

The presets enable `BUILD_TESTING`, place build trees under `build/` with platform-specific names, and stage install trees under `build/<preset>/stage`.

Platform notes:

- on Windows, the `windows-mingw-*` and `windows-msvc-*` presets are exposed
- on Linux, only the `linux-ninja-*` presets are exposed
- the Windows presets intentionally read the following user environment variables instead of hard-coding local install paths:
  - `QT_WIN_MINGW_ROOT`
  - `QT_WIN_MSVC_ROOT`
  - `QT_WIN_MINGW_TOOLS_ROOT`
  - `QT_WIN_NINJA_ROOT`
- Linux presets intentionally do not hard-code a Qt path; they work with a distro Qt install, a shell that already exports `QT_ROOT_DIR`, or `./scripts/build.sh --qt-root <path>`
- when Qt Creator configures the project without using a preset, `CMakeLists.txt` also falls back to `QT_ROOT_DIR` and prepends it to `CMAKE_PREFIX_PATH`
- the MSVC presets also assume Visual Studio 2022 / Build Tools 2022 with the x64 C++ toolchain installed

### Windows Local Environment Setup

On Windows, the checked-in presets can auto-detect a standard `C:\Qt` layout through the helper script. Run it once to persist the variables for future terminals and Qt Creator sessions:

```powershell
.\scripts\setup_windows_qt_env.ps1 -QtRoot 'C:\Qt' -Scope User
```

If your installation differs from the standard layout shown above, pass the matching arguments explicitly. The script writes whichever toolchain paths actually exist, so a contributor can keep only MinGW or only MSVC installed locally and still use the matching preset.

Example:

```powershell
.\scripts\setup_windows_qt_env.ps1 `
  -QtRoot 'D:\Qt' `
  -QtVersion '6.11.0' `
  -MingwToolsDirName 'mingw1310_64' `
  -NinjaDirName 'Ninja' `
  -Scope User
```

For one-off shells, use `-Scope Process` instead.

If you prefer local overrides instead of persistent user environment variables, use `CMakeUserPresets.json.example` as the starting point for a local `CMakeUserPresets.json`.

### Qt Creator / MSVC Troubleshooting

If Qt Creator reports:

```text
MSB1009: Project file does not exist.
```

first check whether CMake generated the MSVC project files:

```powershell
Test-Path .\build\windows-msvc-debug\DevicePilotHMI.vcxproj
Test-Path .\build\windows-msvc-debug\ALL_BUILD.vcxproj
```

If those files exist, the CMake project is configured and the error usually means Qt Creator is invoking MSBuild with a stale or invalid local build/deploy step. In that case:

- in `Projects > Build Settings`, select `DevicePilotHMI` or `ALL_BUILD` as the build target
- in `Run/Deploy Settings`, use the normal desktop run/deploy configuration instead of an Application Manager package deploy step
- remove empty custom build targets from deploy/package steps
- if the kit state is still wrong, close Qt Creator and regenerate the local user configuration by moving aside `CMakeLists.txt.user` and `.qtcreator/CMakeLists.txt.user`, then reopen the project and select the MSVC preset again

If the `.vcxproj` files are missing, reconfigure the preset or do a fresh rebuild after confirming that `QT_WIN_MSVC_ROOT` points to the matching Qt MSVC install, for example `C:\Qt\6.11.0\msvc2022_64`.

## Run

There are two different runtime layouts:

- development build tree
- deployed install/package layout

The executable under `build/<preset>/bin/` is a development artifact. It runs when the process already has access to the matching Qt runtime, plugins, and QML import paths, such as:

- Qt Creator with the preset kit configured
- a shell where the matching Qt environment is already exported

If you want a locally runnable layout that does not depend on your current shell's Qt setup, run the install step and launch from the staged output instead.

Windows:

```powershell
.\scripts\build.ps1 -Preset windows-mingw-debug -Install
```

Then start:

```text
build/windows-mingw-debug/stage/bin/DevicePilotHMI.exe
```

Linux:

```bash
./scripts/build.sh --preset linux-ninja-debug --install --qt-root /home/<user>/Qt/6.11.0/gcc_64
```

Then start:

```text
build/linux-ninja-debug/stage/bin/DevicePilotHMI
```

For a redistributable Windows package, use the packaging script:

```powershell
.\scripts\package_windows.ps1 -Preset windows-mingw-release
```

This creates a zip under `release/`, for example:

```text
release/DevicePilotHMI-windows-mingw-x64.zip
```

Unzip it and run:

```text
bin/DevicePilotHMI.exe
```

Typical usage flow:

1. Start the machine from the dashboard.
2. Watch telemetry rise in the running state.
3. Observe warning, fault, reset, and recovery transitions when thresholds are crossed or faults are reset.
4. Open the event log to inspect runtime and config activity.
5. Open settings to apply a threshold preset or edit thresholds/update interval directly.
6. Apply or revert the draft and confirm persistence across restarts.

## Automated Tests

The repository includes focused C++ tests for the non-UI application layers, including:

- settings validation, JSON codec, file store, manager, draft, apply policy, and session behavior
- machine runtime start, stop, fault, and reset behavior against a controllable fake backend
- simulated backend telemetry, state progression, scenario, and runtime integration behavior
- alarm lifecycle transitions, threshold-triggered warning/fault behavior, and immediate reevaluation after applying threshold changes while running
- fake backend assisted runtime/alarm scenarios across idle, starting, running, fault, reset, and recovery states

The tests are wired through `tests/CMakeLists.txt` and enabled by the checked-in CMake presets.

## Current Limitations

- The runtime is still driven by a simulated backend; there is no real device backend yet.
- The alarm banner still presents one primary active metric at a time; the project does not yet implement a stacked or aggregated multi-alarm operator summary.
- The project still does not model a full industrial alarm system with shelving, suppression, historian integration, or alarm acknowledgment as part of the alarm lifecycle itself.
- The automated test suite currently focuses on core C++ modules and does not cover QML UI behavior end-to-end.
- The project is currently organized as a single application target rather than a split core library plus app shell.

## Development Notes

This codebase is most useful as:

- a Qt/QML architecture practice project
- a small HMI portfolio piece
- a base for further refactoring toward real-device integration, stronger alarm modeling, deeper test coverage, a cleaner core/app split, and portable Linux CI/build coverage

## License

This repository is licensed under the MIT License. See [LICENSE](LICENSE).
