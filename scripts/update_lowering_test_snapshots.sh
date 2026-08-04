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

tmp_dir="$(mktemp -d /tmp/abys-snapshots.XXXXXX)"
trap 'rm -rf "$tmp_dir"' EXIT

mapfile -t inputs < <(
  find "$root_dir/tests/frontend/fixtures" -mindepth 2 -maxdepth 2 -type f -name input.sv -print |
    sort
)

stale=0
failures=0

for input_file in "${inputs[@]}"; do
  test_dir="$(dirname "$input_file")"
  test_name="${test_dir#"$root_dir/"}"
  expected_file="$test_dir/expected.sv"
  candidate="$tmp_dir/$(basename "$test_dir").sv"
  equivalence_log="$tmp_dir/$(basename "$test_dir").equivalence.log"

  if ! "$equivalence_script" "$input_file" top "$candidate" >"$equivalence_log" 2>&1; then
    cat "$equivalence_log" >&2
    echo "EQUIVALENCE CHECK FAILED: $test_name" >&2
    failures=1
    continue
  fi

  if [[ -f "$expected_file" ]] && cmp -s "$candidate" "$expected_file"; then
    echo "current: $test_name"
    continue
  fi

  stale=1
  if $check_only; then
    echo "stale: $test_name"
  else
    cp "$candidate" "$expected_file"
    echo "updated: $test_name"
  fi
done

if [[ $failures -ne 0 ]]; then
  echo "error: failed snapshots were not updated" >&2
  exit 1
fi

if $check_only && [[ $stale -ne 0 ]]; then
  exit 1
fi
