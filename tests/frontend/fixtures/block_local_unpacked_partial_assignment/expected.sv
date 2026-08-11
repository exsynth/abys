module top (
  input [3:0] d_i,
  output  logic [3:0] d_o);



  always @(*)   begin
    logic [3:0] abys_dumper_tmp12 [1:0];
    logic signed [32:0] abys_dumper_tmp8;
    logic signed [32:0] abys_dumper_tmp10;
    logic signed [32:0] abys_dumper_tmp14;
    logic signed [32:0] abys_dumper_tmp16;
    logic [3:0] abys_dumper_tmp17;
    abys_dumper_tmp12[1] = 4'bxxxx;
    abys_dumper_tmp12[0] = 4'bxxxx;
    abys_dumper_tmp8 = 32'sb0;
    abys_dumper_tmp10 = (33'sb1 - abys_dumper_tmp8);
    abys_dumper_tmp12[abys_dumper_tmp10] = d_i;
    abys_dumper_tmp14 = 32'sb1;
    abys_dumper_tmp16 = (33'sb1 - abys_dumper_tmp14);
    abys_dumper_tmp17 = abys_dumper_tmp12[abys_dumper_tmp16];
    d_o = abys_dumper_tmp17;
  end
endmodule
