module top (
  input [3:0] d_i,
  output  logic [3:0] d_o);



  always @(*)   begin
    logic [3:0] abys_dumper_tmp11 [1:0];
    logic [3:0] abys_dumper_tmp4 [1:0];
    logic signed [32:0] abys_dumper_tmp7;
    logic signed [32:0] abys_dumper_tmp9;
    logic signed [32:0] abys_dumper_tmp13;
    logic signed [32:0] abys_dumper_tmp15;
    logic [3:0] abys_dumper_tmp16;
    abys_dumper_tmp4 = '{4'bxxxx, 4'bxxxx};
    abys_dumper_tmp11 = abys_dumper_tmp4;
    abys_dumper_tmp7 = 32'sb0;
    abys_dumper_tmp9 = (33'sb1 - abys_dumper_tmp7);
    abys_dumper_tmp11[abys_dumper_tmp9] = d_i;
    abys_dumper_tmp13 = 32'sb1;
    abys_dumper_tmp15 = (33'sb1 - abys_dumper_tmp13);
    abys_dumper_tmp16 = abys_dumper_tmp11[abys_dumper_tmp15];
    d_o = abys_dumper_tmp16;
  end
endmodule
