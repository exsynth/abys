module top(
  input logic signed [7:0] a,
  input logic [3:0] b,
  output logic signed [8:0] y
);
  assign y = a + b;
endmodule
