module top (
  input [3:0] d_i,
  output  logic [3:0] d_o);



  always @(*)   begin
    logic [3:0] abys_dumper_tmp9 [0:1];
    logic [3:0] abys_dumper_tmp11;
    abys_dumper_tmp9[0] = 4'bxxxx;
    abys_dumper_tmp9[1] = 4'bxxxx;
    abys_dumper_tmp9[32'sb0] = d_i;
    abys_dumper_tmp11 = abys_dumper_tmp9[32'sb1];
    d_o = abys_dumper_tmp11;
  end
endmodule
