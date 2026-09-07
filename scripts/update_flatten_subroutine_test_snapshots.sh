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
equivalence_script="$root_dir/scripts/check_equivalence.sh"
fixture_dir="$root_dir/tests/transform/flatten_subroutine/fixtures"
input_file="$fixture_dir/input.sv"
expected_file="$fixture_dir/expected.sv"

tmp_dir="$(mktemp -d /tmp/abys-flatten-subroutine.XXXXXX)"
trap 'rm -rf "$tmp_dir"' EXIT
candidate="$tmp_dir/expected.sv"
equivalence_log="$tmp_dir/equivalence.log"

if ! env ABYS_COMMANDS='read_slang $env(ABYS_ORIG_SV) -top $env(ABYS_TOP); flatten_subroutine; dump $env(ABYS_LOWERED_SV)' \
  "$equivalence_script" "$input_file" top "$candidate" >"$equivalence_log" 2>&1; then
  cat "$equivalence_log" >&2
  echo "EQUIVALENCE CHECK FAILED: tests/transform/flatten_subroutine/fixtures" >&2
  exit 1
fi

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
