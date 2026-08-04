module top(
  input logic [7:0] a,
  input logic [7:0] b,
  input logic [7:0] c,
  output logic [7:0] y
);
  assign y = (a & b) | c;
endmodule
