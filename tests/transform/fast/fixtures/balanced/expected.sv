module top (
  input a,
  input b,
  input c,
  input d,
  output  logic y);



  assign y = ((b & a) & (d & c));
endmodule
