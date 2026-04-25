param(
    [string]$QtRoot = ""
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$buildDir = Join-Path $repoRoot "build\clang-tidy"

function Resolve-Executable {
    param(
        [Parameter(Mandatory)]
        [string]$CommandName,
        [string[]]$Fallbacks = @()
    )

    $command = Get-Command $CommandName -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }

    foreach ($candidate in $Fallbacks) {
        if (Test-Path -LiteralPath $candidate) {
            return (Resolve-Path -LiteralPath $candidate).Path
        }
    }

    throw "$CommandName was not found in PATH."
}

function Invoke-Step {
    param(
        [Parameter(Mandatory)]
        [string]$Executable,
        [Parameter(Mandatory)]
        [string[]]$Arguments
    )

    Write-Host ""
    Write-Host "> $Executable $($Arguments -join ' ')"
    & $Executable @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed with exit code ${LASTEXITCODE}: $Executable $($Arguments -join ' ')"
    }
}

$llvmPath = "C:\Program Files\LLVM\bin"
if (Test-Path -LiteralPath $llvmPath) {
    $env:PATH = "$llvmPath;$env:PATH"
}

if ([string]::IsNullOrWhiteSpace($QtRoot)) {
    $QtRoot = [Environment]::GetEnvironmentVariable("QT_ROOT_DIR", "Process")
}

if (-not [string]::IsNullOrWhiteSpace($QtRoot)) {
    $resolvedQtRoot = (Resolve-Path -LiteralPath $QtRoot).Path
    $env:QT_ROOT_DIR = $resolvedQtRoot
    $env:CMAKE_PREFIX_PATH = $resolvedQtRoot
    $env:PATH = "$resolvedQtRoot\bin;$env:PATH"
}

if (-not [string]::IsNullOrWhiteSpace($env:QT_WIN_MINGW_TOOLS_ROOT)) {
    $env:PATH = "$env:QT_WIN_MINGW_TOOLS_ROOT\bin;$env:PATH"
}

if (-not [string]::IsNullOrWhiteSpace($env:QT_WIN_NINJA_ROOT)) {
    $env:PATH = "$env:QT_WIN_NINJA_ROOT;$env:PATH"
}

$cmakeExe = Resolve-Executable -CommandName "cmake" -Fallbacks @(
    "C:\Program Files\CMake\bin\cmake.exe",
    "C:\Qt\Tools\CMake_64\bin\cmake.exe"
)
$pythonExe = Resolve-Executable -CommandName "python"
$null = Resolve-Executable -CommandName "ninja"
$null = Resolve-Executable -CommandName "clang-tidy" -Fallbacks @(
    "C:\Program Files\LLVM\bin\clang-tidy.exe"
)

if (Test-Path -LiteralPath $buildDir) {
    Remove-Item -LiteralPath $buildDir -Recurse -Force
}

Invoke-Step -Executable $cmakeExe -Arguments @(
    "-S", $repoRoot,
    "-B", $buildDir,
    "-G", "Ninja",
    "-DCMAKE_BUILD_TYPE=Debug",
    "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
)

Invoke-Step -Executable $pythonExe -Arguments @(
    (Join-Path $repoRoot "scripts\invoke_clang_tidy.py"),
    "--build-dir", $buildDir,
    "--config-file", (Join-Path $repoRoot ".clang-tidy"),
    "--quiet"
)
