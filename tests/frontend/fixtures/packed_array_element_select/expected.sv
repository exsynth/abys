module top (
  input [31:0] data_i,
  input [1:0] index_i,
  input [7:0] update_i,
  output  logic [7:0] selected_o,
  output  logic [31:0] updated_o);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp5;
    abys_dumper_tmp5 = data_i[($signed(10'({1'b0, 1'b0})) + ($signed(10'({1'b0, index_i})) * $signed(10'($signed(5'sb1000))))) +: 8];
    selected_o = abys_dumper_tmp5;
  end
  always @(*)   begin
    logic [31:0] abys_dumper_tmp7;
    abys_dumper_tmp7 = data_i;
    abys_dumper_tmp7[($signed(10'({1'b0, 1'b0})) + ($signed(10'({1'b0, index_i})) * $signed(10'($signed(5'sb1000))))) +: 4'b1000] = update_i;
    updated_o = abys_dumper_tmp7;
  end
endmodule
