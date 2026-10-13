#!/usr/bin/env python3

import argparse
import itertools
import math
import os
import pathlib
import re
import shutil
import struct
import subprocess
import sys
import tempfile
import zlib
from dataclasses import dataclass, field


@dataclass
class Pin:
    name: str
    input_load: float
    rise_block: float
    rise_fanout: float
    fall_block: float
    fall_fanout: float


@dataclass
class Gate:
    name: str
    area: float
    output: str
    expression: str
    pins: list[Pin] = field(default_factory=list)


class ExpressionParser:
    def __init__(self, expression: str, values: dict[str, bool]):
        self.tokens = re.findall(r"[A-Za-z_][A-Za-z0-9_$]*|[01]|[()!'+*^|&]", expression)
        self.position = 0
        self.values = values

    def parse(self) -> bool:
        value = self.parse_or()
        if self.position != len(self.tokens):
            raise ValueError("unsupported Boolean expression")
        return value

    def accept(self, *tokens: str) -> bool:
        if self.position < len(self.tokens) and self.tokens[self.position] in tokens:
            self.position += 1
            return True
        return False

    def parse_or(self) -> bool:
        value = self.parse_xor()
        while self.accept("+", "|"):
            value = self.parse_xor() or value
        return value

    def parse_xor(self) -> bool:
        value = self.parse_and()
        while self.accept("^"):
            value = self.parse_and() != value
        return value

    def parse_and(self) -> bool:
        value = self.parse_unary()
        while self.accept("*", "&"):
            value = self.parse_unary() and value
        return value

    def parse_unary(self) -> bool:
        if self.accept("!"):
            return not self.parse_unary()
        if self.accept("("):
            value = self.parse_or()
            if not self.accept(")"):
                raise ValueError("unclosed parenthesis")
        else:
            if self.position == len(self.tokens):
                raise ValueError("missing operand")
            token = self.tokens[self.position]
            self.position += 1
            if token in ("CONST0", "0"):
                value = False
            elif token in ("CONST1", "1"):
                value = True
            else:
                value = self.values[token]
        while self.accept("'"):
            value = not value
        return value


def read_genlib(path: pathlib.Path) -> list[Gate]:
    gate_pattern = re.compile(
        r"\bGATE\s+(\S+)\s+([-+0-9.eE]+)\s+([A-Za-z_][A-Za-z0-9_$]*)\s*=\s*([^;]+);"
    )
    pin_pattern = re.compile(
        r"\bPIN\s+(\S+)\s+(?:INV|NONINV|UNKNOWN)\s+"
        r"([-+0-9.eE]+)\s+([-+0-9.eE]+)\s+([-+0-9.eE]+)\s+"
        r"([-+0-9.eE]+)\s+([-+0-9.eE]+)\s+([-+0-9.eE]+)"
    )
    gates: list[Gate] = []
    current = None
    for line in path.read_text().splitlines():
        gate_match = gate_pattern.search(line)
        if gate_match:
            current = Gate(
                gate_match.group(1),
                float(gate_match.group(2)),
                gate_match.group(3),
                gate_match.group(4),
            )
            gates.append(current)
        if current is None:
            continue
        for pin_match in pin_pattern.finditer(line):
            current.pins.append(
                Pin(
                    pin_match.group(1),
                    float(pin_match.group(2)),
                    float(pin_match.group(4)),
                    float(pin_match.group(5)),
                    float(pin_match.group(6)),
                    float(pin_match.group(7)),
                )
            )
    return gates


def truth_table(gate: Gate) -> tuple[tuple[str, ...], tuple[bool, ...]]:
    names = tuple(sorted(pin.name for pin in gate.pins))
    values = []
    for assignment in itertools.product((False, True), repeat=len(names)):
        values.append(ExpressionParser(gate.expression, dict(zip(names, assignment))).parse())
    return names, tuple(values)


def collect_cells(gates: list[Gate]) -> dict[str, list[Gate]]:
    cells: dict[str, list[Gate]] = {}
    for gate in gates:
        cells.setdefault(gate.name, []).append(gate)
    return cells


def cell_signature(outputs: list[Gate]):
    return tuple(sorted((gate.output, truth_table(gate)) for gate in outputs))


def time_unit_ps(liberty: str) -> float:
    match = re.search(r'\btime_unit\s*:\s*"([0-9.]+)(fs|ps|ns|us)"', liberty)
    if not match:
        raise ValueError("unsupported or missing Liberty time_unit")
    scales = {"fs": 0.001, "ps": 1.0, "ns": 1000.0, "us": 1_000_000.0}
    return float(match.group(1)) * scales[match.group(2)]


def find_output(cells: dict[str, list[Gate]], cell: str, output: str):
    return next((gate for gate in cells.get(cell, ()) if gate.output == output), None)


def find_pin(gate: Gate, name: str):
    return next((pin for pin in gate.pins if pin.name == name), None)


def scalar_delay(pin: Pin, gain: float) -> float:
    load = 0.01 * gain * pin.input_load
    rise = pin.rise_block + pin.rise_fanout * load
    fall = pin.fall_block + pin.fall_fanout * load
    return 0.5 * (rise + fall)


def write_plot(path: pathlib.Path, delay_pairs: list[tuple[float, float]], correlation: float) -> None:
    size = 720
    margin = 60
    pixels = bytearray([255] * size * size * 3)

    def set_pixel(x: int, y: int, color: tuple[int, int, int]) -> None:
        if 0 <= x < size and 0 <= y < size:
            offset = 3 * (y * size + x)
            pixels[offset : offset + 3] = bytes(color)

    def draw_line(x0: int, y0: int, x1: int, y1: int, color: tuple[int, int, int]) -> None:
        dx = abs(x1 - x0)
        dy = -abs(y1 - y0)
        step_x = 1 if x0 < x1 else -1
        step_y = 1 if y0 < y1 else -1
        error = dx + dy
        while True:
            set_pixel(x0, y0, color)
            if x0 == x1 and y0 == y1:
                break
            doubled_error = 2 * error
            if doubled_error >= dy:
                error += dy
                x0 += step_x
            if doubled_error <= dx:
                error += dx
                y0 += step_y

    abys_delays = [pair[0] for pair in delay_pairs]
    abc_delays = [pair[1] for pair in delay_pairs]
    limit = max(abys_delays + abc_delays) * 1.05
    plot_size = size - 2 * margin
    for division in range(6):
        coordinate = margin + division * plot_size // 5
        draw_line(margin, coordinate, size - margin, coordinate, (225, 225, 225))
        draw_line(coordinate, margin, coordinate, size - margin, (225, 225, 225))
    draw_line(margin, size - margin, size - margin, margin, (100, 100, 100))
    draw_line(margin, margin, margin, size - margin, (0, 0, 0))
    draw_line(margin, size - margin, size - margin, size - margin, (0, 0, 0))
    for abys_delay, abc_delay in delay_pairs:
        x = margin + round(abc_delay / limit * plot_size)
        y = size - margin - round(abys_delay / limit * plot_size)
        for offset_y in range(-3, 4):
            for offset_x in range(-3, 4):
                if offset_x * offset_x + offset_y * offset_y <= 9:
                    set_pixel(x + offset_x, y + offset_y, (31, 119, 180))

    def chunk(kind: bytes, data: bytes) -> bytes:
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data))

    rows = b"".join(b"\0" + pixels[row * size * 3 : (row + 1) * size * 3] for row in range(size))
    description = (
        f"ABC delay on x-axis; Abys delay on y-axis; Pearson r={correlation:.6f}"
    ).encode()
    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", struct.pack(">IIBBBBB", size, size, 8, 2, 0, 0, 0))
    png += chunk(b"tEXt", b"Description\0" + description)
    png += chunk(b"IDAT", zlib.compress(rows))
    png += chunk(b"IEND", b"")
    path.write_bytes(png)


def write_snapshot(path: pathlib.Path, liberty_path: pathlib.Path, genlib_path: pathlib.Path) -> None:
    header = [
        f"# Generated by Abys from {liberty_path.name}.",
        "# This is a derived work; see README.md and LICENSE in this directory.",
    ]
    if (liberty_path.parent / "NOTICE").is_file():
        header.append("# See NOTICE for additional attribution.")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(header) + "\n\n" + genlib_path.read_text())


def compare(
    abys_path: pathlib.Path,
    abc_path: pathlib.Path,
    gain: float,
    unit_ps: float,
    plot_path: pathlib.Path | None,
) -> bool:
    abys_gates = read_genlib(abys_path)
    abc_gates = read_genlib(abc_path)
    abys_cells = collect_cells(abys_gates)
    abc_cells = collect_cells(abc_gates)
    classes: dict[tuple, list[str]] = {}
    for name, outputs in abys_cells.items():
        classes.setdefault(cell_signature(outputs), []).append(name)

    mismatch = False
    errors = []
    delay_pairs: list[tuple[float, float]] = []
    class_delays: dict[tuple, dict[tuple[str, str], tuple[float, float]]] = {}
    compared_outputs = 0
    for abc_gate in abc_gates:
        abys_gate = find_output(abys_cells, abc_gate.name, abc_gate.output)
        if abys_gate is None:
            print(f"missing Abys output: {abc_gate.name}/{abc_gate.output}", file=sys.stderr)
            mismatch = True
            continue
        compared_outputs += 1
        if truth_table(abys_gate) != truth_table(abc_gate):
            print(f"function mismatch: {abc_gate.name}/{abc_gate.output}", file=sys.stderr)
            mismatch = True
        if not math.isclose(abys_gate.area, abc_gate.area, abs_tol=0.011):
            print(
                f"area mismatch: {abc_gate.name}/{abc_gate.output} "
                f"(Abys {abys_gate.area}, ABC {abc_gate.area})",
                file=sys.stderr,
            )
            mismatch = True

        signature = cell_signature(abc_cells[abc_gate.name])
        if signature not in classes:
            print(f"missing Abys cell class: {abc_gate.name}", file=sys.stderr)
            mismatch = True
            continue
        class_delays.setdefault(signature, {})
        for abc_pin in abc_gate.pins:
            delays = []
            for cell_name in classes[signature]:
                class_gate = find_output(abys_cells, cell_name, abc_gate.output)
                if class_gate is None:
                    continue
                class_pin = find_pin(class_gate, abc_pin.name)
                if class_pin is not None:
                    delays.append(scalar_delay(class_pin, gain) * unit_ps)
            if not delays:
                print(
                    f"missing Abys arc: {abc_gate.name}/{abc_gate.output} <- {abc_pin.name}",
                    file=sys.stderr,
                )
                mismatch = True
                continue
            abys_delay = sum(delays) / len(delays)
            if not math.isclose(abc_pin.rise_block, abc_pin.fall_block):
                print(
                    f"ABC scalar delay mismatch: {abc_gate.name}/{abc_gate.output} "
                    f"<- {abc_pin.name} (rise {abc_pin.rise_block}, "
                    f"fall {abc_pin.fall_block})",
                    file=sys.stderr,
                )
                mismatch = True
                continue
            abc_delay = abc_pin.rise_block
            arc = (abc_gate.output, abc_pin.name)
            previous = class_delays[signature].get(arc)
            if previous is not None:
                if not math.isclose(previous[1], abc_delay):
                    print(
                        f"inconsistent ABC class delay: {abc_gate.name}/{abc_gate.output} "
                        f"<- {abc_pin.name} ({previous[1]} and {abc_delay})",
                        file=sys.stderr,
                    )
                    mismatch = True
                continue
            class_delays[signature][arc] = (abys_delay, abc_delay)
            errors.append(abs(abys_delay - abc_delay))
            delay_pairs.append((abys_delay, abc_delay))

    if any(len(outputs) > 1 for outputs in abys_cells.values()):
        print(
            "note: ABC may reuse one output's timing model for other outputs of a "
            "multi-output cell; adder sum outputs can therefore appear as timing outliers"
        )
    print("per-class scalar delays:")
    for signature in sorted(class_delays, key=lambda item: classes[item]):
        if not class_delays[signature]:
            continue
        print(f"  {', '.join(sorted(classes[signature]))}:")
        for (output, input_name), (abys_delay, abc_delay) in sorted(class_delays[signature].items()):
            print(
                f"    {output} <- {input_name}: Abys {abys_delay:.4f} ps, "
                f"ABC {abc_delay:.4f} ps, difference {abs(abys_delay - abc_delay):.4f} ps"
            )
    print(f"compared outputs: {compared_outputs}")
    print(f"compared class timing arcs: {len(errors)}")
    if errors:
        print(f"mean scalar-delay difference: {sum(errors) / len(errors):.4f} ps")
        print(f"maximum scalar-delay difference: {max(errors):.4f} ps")
    correlation = None
    if len(delay_pairs) >= 2:
        abys_mean = sum(pair[0] for pair in delay_pairs) / len(delay_pairs)
        abc_mean = sum(pair[1] for pair in delay_pairs) / len(delay_pairs)
        covariance = sum(
            (abys_delay - abys_mean) * (abc_delay - abc_mean)
            for abys_delay, abc_delay in delay_pairs
        )
        abys_variance = sum((pair[0] - abys_mean) ** 2 for pair in delay_pairs)
        abc_variance = sum((pair[1] - abc_mean) ** 2 for pair in delay_pairs)
        if abys_variance > 0.0 and abc_variance > 0.0:
            correlation = covariance / math.sqrt(abys_variance * abc_variance)
            print(f"Pearson delay correlation: {correlation:.6f}")
    if plot_path is not None:
        if correlation is None:
            raise RuntimeError("at least two varying timing arcs are required for a plot")
        write_plot(plot_path, delay_pairs, correlation)
        print(f"wrote delay plot: {plot_path}")
    return not mismatch


def update_snapshot(
    liberty_path: pathlib.Path,
    abys: pathlib.Path,
    abc: str,
    input_slew: float | None,
    gain: float,
    plot_path: pathlib.Path | None,
) -> bool:
    liberty = liberty_path.read_text()
    unit_ps = time_unit_ps(liberty)
    with tempfile.TemporaryDirectory(prefix="abys-genlib.") as temporary:
        temporary_path = pathlib.Path(temporary)
        abys_genlib = temporary_path / "abys.genlib"
        abc_genlib = temporary_path / "abc.genlib"
        environment = os.environ | {
            "ABYS_INPUT_LIBERTY": str(liberty_path),
            "ABYS_OUTPUT_GENLIB": str(abys_genlib),
        }
        abys_read = "read_liberty $env(ABYS_INPUT_LIBERTY)"
        abc_read = f"read_lib -G {gain} -a {liberty_path}"
        if input_slew is not None:
            abys_read += f" -input_slew {input_slew / unit_ps}"
            abc_read = f"read_lib -S {input_slew} -G {gain} -a {liberty_path}"
        subprocess.run(
            [
                abys,
                "--commands",
                f"{abys_read}; write_genlib $env(ABYS_OUTPUT_GENLIB)",
            ],
            check=True,
            env=environment,
        )
        snapshot_path = liberty_path.parent / "expected.genlib"
        write_snapshot(snapshot_path, liberty_path, abys_genlib)
        print(f"wrote Abys GENLIB snapshot: {snapshot_path}")
        subprocess.run(
            [
                abc,
                "-q",
                f"{abc_read}; write_genlib {abc_genlib}",
            ],
            check=True,
        )
        return compare(abys_genlib, abc_genlib, gain, unit_ps, plot_path)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Update Abys GENLIB snapshots and compare them with ABC."
    )
    parser.add_argument("--input-slew-ps", type=float)
    parser.add_argument("--gain", type=float, default=100.0)
    parser.add_argument("--plot-dir", type=pathlib.Path, metavar="DIRECTORY")
    arguments = parser.parse_args()

    root = pathlib.Path(__file__).resolve().parent.parent
    fixture_root = root / "tests/fixtures/libraries"
    liberty_paths = [
        fixture_dir / f"{fixture_dir.name}.lib"
        for fixture_dir in sorted(fixture_root.iterdir())
        if fixture_dir.is_dir()
    ]

    abys = pathlib.Path(os.environ.get("ABYS", root / "build/abys"))
    abc = os.environ.get("ABC", "yosys-abc")
    if not abys.is_file():
        print(f"error: Abys executable not found: {abys}", file=sys.stderr)
        return 2
    if shutil.which(abc) is None:
        print(f"error: ABC executable not found: {abc}", file=sys.stderr)
        return 2

    success = True
    for liberty_path in liberty_paths:
        print(f"\n{liberty_path.parent.name}:")
        plot_path = None
        if arguments.plot_dir is not None:
            plot_path = arguments.plot_dir.resolve() / f"{liberty_path.parent.name}.png"
            plot_path.parent.mkdir(parents=True, exist_ok=True)
        success &= update_snapshot(
            liberty_path,
            abys,
            abc,
            arguments.input_slew_ps,
            arguments.gain,
            plot_path,
        )
    return 0 if success else 1


if __name__ == "__main__":
    sys.exit(main())
