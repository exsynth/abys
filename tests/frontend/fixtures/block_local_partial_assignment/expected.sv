module top (
  input [3:0] d_i,
  output  logic [7:0] d_o);



  always @(*)   begin
    logic [2:0] abys_dumper_tmp5;
    logic [7:0] abys_dumper_tmp7;
    abys_dumper_tmp5 = (1'b0 + 3'b100);
    abys_dumper_tmp7 = 8'bxxxxxxxx;
    abys_dumper_tmp7[abys_dumper_tmp5 +: 3'b100] = d_i;
    d_o = abys_dumper_tmp7;
  end
endmodule
