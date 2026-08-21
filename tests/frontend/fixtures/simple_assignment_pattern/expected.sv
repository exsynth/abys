module top (
  input [7:0] a_i,
  input [7:0] b_i,
  output  logic [15:0] packed_o,
  output  logic [7:0] unpacked_o [0:1]);



  always @(*)   begin
    logic [15:0] abys_dumper_tmp4;
    abys_dumper_tmp4 = {a_i, b_i};
    packed_o = abys_dumper_tmp4;
  end
  always @(*)   begin
    unpacked_o[0] = a_i;
    unpacked_o[1] = b_i;
  end
endmodule
