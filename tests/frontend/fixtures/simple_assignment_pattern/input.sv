module top (
  input  logic [7:0] a_i,
  input  logic [7:0] b_i,
  output logic [1:0][7:0] packed_o,
  output logic [7:0] unpacked_o [0:1]
);
  assign packed_o = '{a_i, b_i};
  assign unpacked_o = '{a_i, b_i};
endmodule
