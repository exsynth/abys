module top (
  input [23:0] data_i,
  input [15:0] update_i,
  output  logic [15:0] data_o,
  output  logic [23:0] updated_o);



  always @(*)   begin
    logic [15:0] abys_dumper_tmp3;
    abys_dumper_tmp3 = data_i[1'b0 +: 16];
    data_o = abys_dumper_tmp3;
  end
  always @(*)   begin
    logic abys_dumper_tmp4;
    logic [23:0] abys_dumper_tmp6;
    abys_dumper_tmp4 = (1'b0 + 1'b0);
    abys_dumper_tmp6 = data_i;
    abys_dumper_tmp6[abys_dumper_tmp4 +: 5'b10000] = update_i;
    updated_o = abys_dumper_tmp6;
  end
endmodule
