#!/usr/bin/env bash
set -euo pipefail

source "$(dirname "${BASH_SOURCE[0]}")/update_snapshot_helpers.sh"
snapshot_begin map "$@"

fixture_dir="$snapshot_root_dir/tests/transform/map/fixtures"
mapfile -t expecteds < <(find "$fixture_dir" -type f -name expected.sv -print | sort)
for expected_file in "${expecteds[@]}"; do
  test_dir="$(dirname "$expected_file")"
  relative_name="${test_dir#"$fixture_dir"/}"
  snapshot_check "$test_dir/input.sv" "$expected_file" \
    "${test_dir#"$snapshot_root_dir"/}" "$relative_name" \
    'read_liberty $env(ABYS_GATE_LIBERTY); read_slang $env(ABYS_ORIG_SV) -top $env(ABYS_TOP); aig; map; dump $env(ABYS_LOWERED_SV)' \
    "$test_dir/library.lib"
done

mapfile -t bus_expecteds < <(find "$fixture_dir" -type f -name expected_bus.sv -print | sort)
for expected_file in "${bus_expecteds[@]}"; do
  test_dir="$(dirname "$expected_file")"
  relative_name="${test_dir#"$fixture_dir"/}"
  snapshot_check "$test_dir/input.sv" "$expected_file" \
    "${test_dir#"$snapshot_root_dir"/} (-bus)" "$relative_name.bus" \
    'read_liberty $env(ABYS_GATE_LIBERTY); read_slang $env(ABYS_ORIG_SV) -top $env(ABYS_TOP); aig -bus; map; dump $env(ABYS_LOWERED_SV)' \
    "$test_dir/library.lib"
done

snapshot_finish
