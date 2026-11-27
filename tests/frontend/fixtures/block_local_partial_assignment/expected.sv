module top (
  input [3:0] d_i,
  output  logic [7:0] d_o);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp4;
    abys_dumper_tmp4 = 8'bxxxxxxxx;
    abys_dumper_tmp4[3'b100 +: 3'b100] = d_i;
    d_o = abys_dumper_tmp4;
  end
endmodule
