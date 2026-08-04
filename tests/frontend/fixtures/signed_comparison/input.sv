module top(
  input logic signed [7:0] a,
  input logic signed [7:0] b,
  output logic y
);
  assign y = a < b;
endmodule
