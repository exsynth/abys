#!/usr/bin/env bash
set -euo pipefail

root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
abys_bin="$root_dir/build/abys"

if [[ $# -lt 1 || $# -gt 3 ]]; then
  echo "usage: $0 ORIGINAL_SYSTEMVERILOG [TOP_MODULE] [LOWERED_SYSTEMVERILOG]" >&2
  exit 2
fi

orig_sv="$1"
top="${2:-}"

if [[ ! -x "$abys_bin" ]]; then
  echo "error: abys binary not found or not executable: $abys_bin" >&2
  exit 2
fi

if [[ ! -f "$orig_sv" ]]; then
  echo "error: original SystemVerilog file not found: $orig_sv" >&2
  exit 2
fi
orig_sv="$(realpath "$orig_sv")"

yosys_bin="${YOSYS:-yosys}"

tmp_dir="$(mktemp -d /tmp/abys-equivalence.XXXXXX)"
lowered_sv="${3:-$tmp_dir/lowered.sv}"
equiv_ys="$tmp_dir/equiv.ys"
gold_v="$tmp_dir/gold.v"
gate_v="$tmp_dir/gate.v"
equiv_v="$tmp_dir/equiv.v"
emit_log="$tmp_dir/abys_emit.log"
equiv_log="$tmp_dir/yosys_equiv.log"
orig_yosys_v="$tmp_dir/original_no_stop.v"

normalize_passes="$(cat <<'EOF'
proc
flatten
opt
memory
opt
techmap
opt
delete t:$print
EOF
)"

sed -E -e 's/\$(stop|finish)[[:space:]]*(\([^;]*\))?[[:space:]]*;/begin end/g' \
  -e '/\$readmemh[[:space:]]*[(]/s|^|// |' \
  "$orig_sv" >"$orig_yosys_v"

if [[ -n "$top" ]]; then
  read_orig_cmd="read_slang --ignore-timing --ignore-assertions --ignore-initial --top $top $orig_yosys_v"
  read_gate_cmd="read_slang --ignore-timing --ignore-assertions --ignore-initial --top $top $lowered_sv"
else
  read_orig_cmd="read_slang --ignore-timing --ignore-assertions --ignore-initial $orig_yosys_v"
  read_gate_cmd="read_slang --ignore-timing --ignore-assertions --ignore-initial $lowered_sv"
fi

cat >"$equiv_ys" <<EOF
$read_orig_cmd
$normalize_passes
rename -top gold
design -stash gold
design -reset

$read_gate_cmd
$normalize_passes
rename -top gate
design -stash gate
design -reset

design -copy-from gold gold
design -copy-from gate gate
select gold
write_verilog -selected "$gold_v"
select gate
write_verilog -selected "$gate_v"
equiv_make gold gate equiv
select equiv
async2sync
opt
equiv_simple
equiv_induct -seq 8
select equiv
write_verilog -selected "$equiv_v"
equiv_status -assert equiv
EOF

if ! "$abys_bin" emit "$orig_sv" >"$lowered_sv" 2>"$emit_log"; then
  echo "fail: abys emit failed; tmp: $tmp_dir; log: $emit_log" >&2
  exit 1
fi

if ! "$yosys_bin" -m slang -q "$equiv_ys" >"$equiv_log" 2>&1; then
  echo "fail: Yosys/slang equivalence failed; tmp: $tmp_dir; log: $equiv_log" >&2
  exit 1
fi

rm -rf "$tmp_dir"
echo "ok: Yosys/slang equivalence passed"
