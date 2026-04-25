#!/usr/bin/env bash

set -euo pipefail

qt_root="${1:-${QT_ROOT_DIR:-}}"
repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
build_dir="$repo_root/build/clang-tidy"

if [[ -n "$qt_root" ]]; then
    if [[ ! -d "$qt_root" ]]; then
        echo "Qt directory not found: $qt_root" >&2
        exit 1
    fi

    export QT_ROOT_DIR="$qt_root"
    export CMAKE_PREFIX_PATH="$qt_root${CMAKE_PREFIX_PATH:+:$CMAKE_PREFIX_PATH}"
    export PATH="$qt_root/bin:$PATH"
fi

for command in cmake ninja clang-tidy python3; do
    if ! command -v "$command" >/dev/null 2>&1; then
        echo "$command was not found in PATH." >&2
        exit 1
    fi
done

rm -rf "$build_dir"

cmake -S "$repo_root" -B "$build_dir" -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

python3 "$repo_root/scripts/invoke_clang_tidy.py" \
    --build-dir "$build_dir" \
    --config-file "$repo_root/.clang-tidy" \
    --quiet
