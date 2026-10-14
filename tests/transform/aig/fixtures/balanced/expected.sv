module top (
  input a,
  input b,
  input c,
  input d,
  output  logic y);



  assign y = (((a & b) & c) & d);
endmodule
