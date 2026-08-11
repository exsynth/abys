module top (
  output  logic [31:0] width_o);



  always @(*)   begin
    logic [31:0] abys_dumper_tmp3;
    abys_dumper_tmp3 = 32'sb1000;
    width_o = abys_dumper_tmp3;
  end
endmodule
