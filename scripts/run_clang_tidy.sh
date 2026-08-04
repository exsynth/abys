#!/usr/bin/env bash
set -euo pipefail

if ! command -v clang-tidy >/dev/null 2>&1; then
  echo "clang-tidy not found" >&2
  exit 1
fi

if [ $# -lt 1 ]; then
  echo "Usage: $0 /path/to/compile_commands.json" >&2
  exit 1
fi

compdb=$1
if [ ! -f "$compdb" ]; then
  echo "compile_commands.json not found at $compdb" >&2
  exit 1
fi

mapfile -t files < <(
  git ls-files 'src/*.cpp' 'src/**/*.cpp' 'tests/*.cpp' 'tests/**/*.cpp' \
    ':(exclude)tests/third_party/**' |
    while IFS= read -r file; do
      [[ -f "$file" ]] && printf '%s\n' "$file"
    done
)
if [ ${#files[@]} -eq 0 ]; then
  echo "No source files to lint."
  exit 0
fi

clang-tidy -p "$(dirname "$compdb")" -header-filter="^$PWD/include/" --quiet "${files[@]}"
