module top (
  input [3:0] a,
  input [3:0] b,
  input [3:0] c,
  input [3:0] d,
  output  logic [3:0] y);



  assign y = ((b & a) & (d & c));
endmodule
