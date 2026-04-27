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