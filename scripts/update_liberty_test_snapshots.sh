#!/usr/bin/env bash
set -euo pipefail

root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
abys_bin="$root_dir/build/abys"
openroad_bin="${OPENROAD:-openroad}"
fixture_root="$root_dir/tests/fixtures/libraries"
check_only=false

if [[ $# -eq 1 && "$1" == "--check" ]]; then
  check_only=true
elif [[ $# -ne 0 ]]; then
  echo "usage: $0 [--check]" >&2
  exit 2
fi

if [[ ! -x "$abys_bin" ]]; then
  echo "error: abys binary not found or not executable: $abys_bin" >&2
  exit 2
fi
if ! command -v "$openroad_bin" >/dev/null; then
  echo "error: OpenROAD executable not found: $openroad_bin" >&2
  exit 2
fi

tmp_dir="$(mktemp -d /tmp/abys-liberty.XXXXXX)"
trap 'rm -rf "$tmp_dir"' EXIT

cat >"$tmp_dir/report.tcl" <<'EOF'
read_liberty $env(ABYS_LIBERTY)
set lines {}
foreach library [get_libs *] {
  lappend lines [list library [get_property $library name] \
      [get_property $library full_name]]
}
foreach cell [get_lib_cells *] {
  set cell_name [get_property $cell name]
  lappend lines [list cell $cell_name \
      [get_property $cell base_name] \
      [get_property $cell full_name] \
      [get_property [get_property $cell library] name] \
      [get_property $cell area] \
      [get_property $cell dont_use] \
      [get_property $cell is_buffer] \
      [get_property $cell is_inverter] \
      [get_property $cell is_memory]]
  foreach pin [get_lib_pins -of_objects $cell] {
    lappend lines [list pin $cell_name \
        [get_property $pin name] \
        [get_property $pin full_name] \
        [get_property [get_property $pin lib_cell] name] \
        [get_property $pin direction] \
        [get_property $pin capacitance] \
        [get_property $pin drive_resistance] \
        [get_property $pin drive_resistance_max_fall] \
        [get_property $pin drive_resistance_max_rise] \
        [get_property $pin drive_resistance_min_fall] \
        [get_property $pin drive_resistance_min_rise] \
        [get_property $pin intrinsic_delay] \
        [get_property $pin intrinsic_delay_max_fall] \
        [get_property $pin intrinsic_delay_max_rise] \
        [get_property $pin intrinsic_delay_min_fall] \
        [get_property $pin intrinsic_delay_min_rise] \
        [get_property $pin is_register_clock]]
  }
}
set output [open $env(ABYS_LIBERTY_REPORT) w]
foreach line [lsort $lines] {
  puts $output $line
}
close $output
EOF

stale=0
mapfile -t fixture_dirs < <(find "$fixture_root" -mindepth 1 -maxdepth 1 -type d -print | sort)
for fixture_dir in "${fixture_dirs[@]}"; do
  fixture_name="$(basename "$fixture_dir")"
  input_file="$fixture_dir/$fixture_name.lib"
  expected_file="$fixture_dir/abys_normalized.lib"
  candidate="$tmp_dir/$fixture_name/abys_normalized.lib"
  writer_output="$tmp_dir/$fixture_name/writer.lib"
  original_report="$tmp_dir/$fixture_name/original.report"
  normalized_report="$tmp_dir/$fixture_name/normalized.report"
  mkdir -p "$(dirname "$candidate")"

  ABYS_INPUT_LIBERTY="$input_file" ABYS_OUTPUT_LIBERTY="$writer_output" \
    "$abys_bin" --commands \
      'read_liberty $env(ABYS_INPUT_LIBERTY); write_liberty $env(ABYS_OUTPUT_LIBERTY)' \
      >/dev/null

  {
    awk '/^[[:space:]]*library[[:space:]]*\(/ { exit } { sub(/[[:space:]]+$/, ""); print }' "$input_file"
    cat <<'EOF'
/*
 Modified by Abys: normalized from the original Liberty library.
*/

EOF
    cat "$writer_output"
  } >"$candidate"

  ABYS_LIBERTY="$input_file" ABYS_LIBERTY_REPORT="$original_report" \
    "$openroad_bin" -no_init -exit "$tmp_dir/report.tcl" >/dev/null
  ABYS_LIBERTY="$candidate" ABYS_LIBERTY_REPORT="$normalized_report" \
    "$openroad_bin" -no_init -exit "$tmp_dir/report.tcl" >/dev/null

  if ! diff -u "$original_report" "$normalized_report"; then
    echo "error: OpenROAD reads different cell-library data from normalized $fixture_name" >&2
    exit 1
  fi

  if [[ -f "$expected_file" ]] && cmp -s "$candidate" "$expected_file"; then
    echo "current: tests/fixtures/libraries/$fixture_name/abys_normalized.lib"
  elif $check_only; then
    stale=1
    echo "stale: tests/fixtures/libraries/$fixture_name/abys_normalized.lib"
  else
    cp "$candidate" "$expected_file"
    echo "updated: tests/fixtures/libraries/$fixture_name/abys_normalized.lib"
  fi
done

if [[ $stale -ne 0 ]]; then
  exit 1
fi
