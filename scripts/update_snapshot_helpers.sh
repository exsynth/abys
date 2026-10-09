#!/usr/bin/env bash

snapshot_begin() {
  local prefix="$1"
  shift
  snapshot_check_only=false
  if [[ $# -eq 1 && "$1" == "--check" ]]; then
    snapshot_check_only=true
  elif [[ $# -ne 0 ]]; then
    echo "usage: $0 [--check]" >&2
    exit 2
  fi

  snapshot_root_dir="$(cd "$(dirname "${BASH_SOURCE[1]}")/.." && pwd)"
  snapshot_equivalence_script="$snapshot_root_dir/scripts/check_equivalence.sh"
  snapshot_tmp_dir="$(mktemp -d "/tmp/abys-$prefix.XXXXXX")"
  trap 'rm -rf "$snapshot_tmp_dir"' EXIT
  snapshot_stale=0
  snapshot_failures=0
}

snapshot_check() {
  local input_file="$1"
  local expected_file="$2"
  local test_name="$3"
  local candidate_name="$4"
  local commands="$5"
  local gate_liberty="${6:-}"
  local candidate="$snapshot_tmp_dir/$candidate_name.sv"
  local equivalence_log="$snapshot_tmp_dir/$candidate_name.equivalence.log"
  mkdir -p "$(dirname "$candidate")"

  if ! env ABYS_COMMANDS="$commands" ABYS_GATE_LIBERTY="$gate_liberty" \
    "$snapshot_equivalence_script" "$input_file" top "$candidate" >"$equivalence_log" 2>&1; then
    cat "$equivalence_log" >&2
    echo "EQUIVALENCE CHECK FAILED: $test_name" >&2
    snapshot_failures=1
    return
  fi

  if [[ -f "$expected_file" ]] && cmp -s "$candidate" "$expected_file"; then
    echo "current: $test_name"
  elif $snapshot_check_only; then
    snapshot_stale=1
    echo "stale: $test_name"
  else
    snapshot_stale=1
    cp "$candidate" "$expected_file"
    echo "updated: $test_name"
  fi
}

snapshot_finish() {
  if [[ $snapshot_failures -ne 0 ]]; then
    echo "error: failed snapshots were not updated" >&2
    exit 1
  fi
  if $snapshot_check_only && [[ $snapshot_stale -ne 0 ]]; then
    exit 1
  fi
}
