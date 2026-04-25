#!/usr/bin/env python3
"""Run clang-tidy on project-owned translation units from compile_commands.json."""

from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path


SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".cxx"}


def resolve_from_repo(repo_root: Path, value: str) -> Path:
    path = Path(value)
    if not path.is_absolute():
        path = repo_root / path
    return path.resolve(strict=False)


def resolve_compile_entry_file(entry: dict[str, str]) -> Path:
    file_path = Path(entry["file"])
    if not file_path.is_absolute():
        file_path = Path(entry["directory"]) / file_path
    return file_path.resolve(strict=False)


def path_is_under(path: Path, root: Path) -> bool:
    try:
        return os.path.commonpath(
            [os.path.normcase(os.fspath(path)), os.path.normcase(os.fspath(root))]
        ) == os.path.normcase(os.fspath(root))
    except ValueError:
        return False


def collect_source_entries(
    compile_db: list[dict[str, str]], source_roots: list[Path]
) -> list[dict[str, str]]:
    seen: set[str] = set()
    source_entries: list[dict[str, str]] = []

    for entry in compile_db:
        if "file" not in entry or "directory" not in entry:
            continue

        file_path = resolve_compile_entry_file(entry)
        if file_path.suffix.lower() not in SOURCE_SUFFIXES:
            continue

        if not any(path_is_under(file_path, source_root) for source_root in source_roots):
            continue

        key = os.path.normcase(os.fspath(file_path))
        if key in seen:
            continue

        seen.add(key)
        source_entry = dict(entry)
        source_entry["file"] = os.fspath(file_path)
        source_entries.append(source_entry)

    return source_entries


def make_path_regex(path: Path) -> str:
    return re.escape(path.as_posix()).replace("/", r"[\\/]")


def make_header_filter(source_roots: list[Path]) -> str:
    return "|".join(make_path_regex(source_root) + r"[\\/].*" for source_root in source_roots)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Run clang-tidy on real project sources listed in a CMake compile database. "
            "Generated Qt files are skipped by filtering to source roots such as src/."
        )
    )
    parser.add_argument("--build-dir", default="build/clang-tidy")
    parser.add_argument("--config-file", default=".clang-tidy")
    parser.add_argument("--source-root", action="append", default=["src"])
    parser.add_argument("--clang-tidy", default=os.environ.get("CLANG_TIDY", "clang-tidy"))
    parser.add_argument(
        "--header-filter",
        default="",
        help=(
            "Regex for headers that may emit diagnostics. Defaults to the selected source roots, "
            "so Qt, compiler, and other non-project headers stay outside the reported warning set."
        ),
    )
    parser.add_argument("--warnings-as-errors", default="*")
    parser.add_argument("--quiet", action="store_true")
    parser.add_argument("--dry-run", action="store_true")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    repo_root = Path(__file__).resolve().parents[1]
    build_dir = resolve_from_repo(repo_root, args.build_dir)
    config_file = resolve_from_repo(repo_root, args.config_file)
    compile_commands = build_dir / "compile_commands.json"
    source_roots = [resolve_from_repo(repo_root, source_root) for source_root in args.source_root]

    if not compile_commands.is_file():
        print(f"Missing compile database: {compile_commands}", file=sys.stderr)
        return 2

    with compile_commands.open("r", encoding="utf-8") as file:
        compile_db = json.load(file)

    source_entries = collect_source_entries(compile_db, source_roots)
    source_files = [resolve_compile_entry_file(entry) for entry in source_entries]
    if not source_files:
        print("No project source files found for clang-tidy.", file=sys.stderr)
        print(f"Compile database: {compile_commands}", file=sys.stderr)
        print("Source roots:", file=sys.stderr)
        for source_root in source_roots:
            print(f"  {source_root}", file=sys.stderr)
        return 2

    print(f"Running clang-tidy on {len(source_files)} source files from {compile_commands}")
    if args.dry_run:
        for source_file in source_files:
            print(source_file)
        return 0

    # clang-tidy analyzes every compile database entry that matches a file.
    # CMake lists shared .cpp files once per target, so filter to one command per source
    # to avoid repeated diagnostics from generated test targets.
    with tempfile.TemporaryDirectory(prefix="clang-tidy-", dir=build_dir) as filtered_build_dir:
        filtered_compile_commands = Path(filtered_build_dir) / "compile_commands.json"
        with filtered_compile_commands.open("w", encoding="utf-8") as file:
            json.dump(source_entries, file, indent=2)
            file.write("\n")

        return run_clang_tidy(args, Path(filtered_build_dir), config_file, source_roots, source_files)


def run_clang_tidy(
    args: argparse.Namespace,
    build_dir: Path,
    config_file: Path,
    source_roots: list[Path],
    source_files: list[Path],
) -> int:
    command = [
        args.clang_tidy,
        "-p",
        str(build_dir),
        f"--config-file={config_file}",
        f"--header-filter={args.header_filter or make_header_filter(source_roots)}",
        "--extra-arg=-w",
    ]
    if args.warnings_as_errors:
        command.append(f"--warnings-as-errors={args.warnings_as_errors}")
    if args.quiet:
        command.append("--quiet")
    command.extend(str(source_file) for source_file in source_files)

    try:
        completed = subprocess.run(command, check=False)
    except FileNotFoundError:
        print(f"Unable to find clang-tidy executable: {args.clang_tidy}", file=sys.stderr)
        return 2

    return completed.returncode


if __name__ == "__main__":
    raise SystemExit(main())
