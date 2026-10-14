#!/usr/bin/env bash
set -euo pipefail

source "$(dirname "${BASH_SOURCE[0]}")/update_snapshot_helpers.sh"
snapshot_begin fast "$@"

fixture_dir="$snapshot_root_dir/tests/transform/fast/fixtures"
mapfile -t inputs < <(find "$fixture_dir" -type f -name input.sv -print | sort)
for input_file in "${inputs[@]}"; do
  test_dir="$(dirname "$input_file")"
  relative_name="${test_dir#"$fixture_dir"/}"
  snapshot_check "$input_file" "$test_dir/expected.sv" \
    "${test_dir#"$snapshot_root_dir"/}" "$relative_name" \
    'read_slang $env(ABYS_ORIG_SV) -top $env(ABYS_TOP); aig; fast; dump $env(ABYS_LOWERED_SV)'
done

mapfile -t bus_expecteds < <(find "$fixture_dir" -type f -name expected_bus.sv -print | sort)
for expected_file in "${bus_expecteds[@]}"; do
  test_dir="$(dirname "$expected_file")"
  relative_name="${test_dir#"$fixture_dir"/}"
  snapshot_check "$test_dir/input.sv" "$expected_file" \
    "${test_dir#"$snapshot_root_dir"/} (-bus)" "$relative_name.bus" \
    'read_slang $env(ABYS_ORIG_SV) -top $env(ABYS_TOP); aig -bus; fast; dump $env(ABYS_LOWERED_SV)'
done

snapshot_finish
