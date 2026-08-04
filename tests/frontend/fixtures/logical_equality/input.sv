module top(
  input logic valid,
  input logic [7:0] a,
  input logic [7:0] b,
  output logic y
);
  assign y = valid && (a == b);
endmodule
