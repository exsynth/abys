module top (
  input [23:0] data_i,
  input [23:0] ascending_i,
  input [15:0] update_i,
  input base_i,
  output  logic [15:0] up_o,
  output  logic [15:0] down_o,
  output  logic [15:0] ascending_up_o,
  output  logic [15:0] ascending_down_o,
  output  logic [23:0] updated_up_o,
  output  logic [23:0] updated_down_o);



  always @(*)   begin
    logic [15:0] abys_dumper_tmp5;
    abys_dumper_tmp5 = data_i[($signed(9'({1'b0, 1'b0})) + ($signed(9'({1'b0, base_i})) * $signed(9'($signed(5'sb1000))))) +: 16];
    up_o = abys_dumper_tmp5;
  end
  always @(*)   begin
    logic [15:0] abys_dumper_tmp10;
    logic [31:0] abys_dumper_tmp4;
    logic [31:0] abys_dumper_tmp6;
    logic [31:0] abys_dumper_tmp7;
    abys_dumper_tmp4 = base_i;
    abys_dumper_tmp6 = 32'sb1;
    abys_dumper_tmp7 = (abys_dumper_tmp4 + abys_dumper_tmp6);
    abys_dumper_tmp10 = data_i[($signed(40'($signed(-5'sb1000))) + ($signed(40'({1'b0, abys_dumper_tmp7})) * $signed(40'($signed(5'sb1000))))) +: 16];
    down_o = abys_dumper_tmp10;
  end
  always @(*)   begin
    logic [15:0] abys_dumper_tmp6;
    abys_dumper_tmp6 = ascending_i[($signed(9'({1'b0, 4'b1000})) + ($signed(9'({1'b0, base_i})) * $signed(9'($signed(-5'sb1000))))) +: 16];
    ascending_up_o = abys_dumper_tmp6;
  end
  always @(*)   begin
    logic [15:0] abys_dumper_tmp10;
    logic [31:0] abys_dumper_tmp4;
    logic [31:0] abys_dumper_tmp6;
    logic [31:0] abys_dumper_tmp7;
    abys_dumper_tmp4 = base_i;
    abys_dumper_tmp6 = 32'sb1;
    abys_dumper_tmp7 = (abys_dumper_tmp4 + abys_dumper_tmp6);
    abys_dumper_tmp10 = ascending_i[($signed(40'({1'b0, 5'b10000})) + ($signed(40'({1'b0, abys_dumper_tmp7})) * $signed(40'($signed(-5'sb1000))))) +: 16];
    ascending_down_o = abys_dumper_tmp10;
  end
  always @(*)   begin
    logic [31:0] abys_dumper_tmp8;
    logic [31:0] abys_dumper_tmp10;
    logic [31:0] abys_dumper_tmp11;
    logic [23:0] abys_dumper_tmp15;
    logic [23:0] abys_dumper_tmp7;
    abys_dumper_tmp8 = base_i;
    abys_dumper_tmp10 = 32'sb1;
    abys_dumper_tmp11 = (abys_dumper_tmp8 + abys_dumper_tmp10);
    abys_dumper_tmp15 = data_i;
    abys_dumper_tmp15[($signed(40'($signed(-5'sb1000))) + ($signed(40'({1'b0, abys_dumper_tmp11})) * $signed(40'($signed(5'sb1000))))) +: 5'b10000] = update_i;
    abys_dumper_tmp7 = data_i;
    abys_dumper_tmp7[($signed(9'({1'b0, 1'b0})) + ($signed(9'({1'b0, base_i})) * $signed(9'($signed(5'sb1000))))) +: 5'b10000] = update_i;
    updated_down_o = abys_dumper_tmp15;
    updated_up_o = abys_dumper_tmp7;
  end
endmodule
