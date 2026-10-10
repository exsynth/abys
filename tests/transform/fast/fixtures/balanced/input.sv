module top(
    input  logic a,
    input  logic b,
    input  logic c,
    input  logic d,
    output logic y
);
  assign y = a & b & c & d;
endmodule
