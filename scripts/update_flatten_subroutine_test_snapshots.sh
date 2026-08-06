#!/usr/bin/env bash
set -euo pipefail

check_only=false
if [[ $# -eq 1 && "$1" == "--check" ]]; then
  check_only=true
elif [[ $# -ne 0 ]]; then
  echo "usage: $0 [--check]" >&2
  exit 2
fi

root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
abys_bin="$root_dir/build/abys"
fixture_dir="$root_dir/tests/transform/flatten_subroutine/fixtures"
input_file="$fixture_dir/input.sv"
expected_file="$fixture_dir/expected.sv"

if [[ ! -x "$abys_bin" ]]; then
  echo "error: abys binary not found or not executable: $abys_bin" >&2
  exit 2
fi

tmp_dir="$(mktemp -d /tmp/abys-flatten-subroutine.XXXXXX)"
trap 'rm -rf "$tmp_dir"' EXIT
candidate="$tmp_dir/expected.sv"

env ABYS_INPUT="$input_file" ABYS_OUTPUT="$candidate" \
  "$abys_bin" --commands \
  'read_slang $env(ABYS_INPUT) -top top; flatten_subroutine; dump $env(ABYS_OUTPUT)'

if [[ -f "$expected_file" ]] && cmp -s "$candidate" "$expected_file"; then
  echo "current: tests/transform/flatten_subroutine/fixtures"
  exit 0
fi

if $check_only; then
  echo "stale: tests/transform/flatten_subroutine/fixtures"
  exit 1
fi

cp "$candidate" "$expected_file"
echo "updated: tests/transform/flatten_subroutine/fixtures"
