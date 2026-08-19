module top (
  input [31:0] data_i,
  input [1:0] index_i,
  input [7:0] update_i,
  output  logic [7:0] selected_o,
  output  logic [31:0] updated_o);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp6;
    logic [31:0] abys_dumper_tmp5;
    abys_dumper_tmp5 = (index_i * 32'd8);
    abys_dumper_tmp6 = data_i[abys_dumper_tmp5 +: 8];
    selected_o = abys_dumper_tmp6;
  end
  always @(*)   begin
    logic [31:0] abys_dumper_tmp6;
    logic [31:0] abys_dumper_tmp7;
    logic [31:0] abys_dumper_tmp9;
    abys_dumper_tmp6 = (index_i * 32'd8);
    abys_dumper_tmp7 = (1'b0 + abys_dumper_tmp6);
    abys_dumper_tmp9 = data_i;
    abys_dumper_tmp9[abys_dumper_tmp7 +: 4'b1000] = update_i;
    updated_o = abys_dumper_tmp9;
  end
endmodule
