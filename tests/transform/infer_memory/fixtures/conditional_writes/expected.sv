module top (
  input clk,
  input write_enable,
  input [1:0] write_index,
  input [1:0] read_index,
  input [7:0] write_data,
  output  logic [7:0] read_data_if,
  output  logic [7:0] read_data_else);

  logic [7:0] memory_if [0:3];
  logic [7:0] memory_else [0:3];
  logic [1:0] abys_transformer_tmp0;
  logic abys_transformer_tmp1;
  logic [7:0] abys_transformer_tmp2;
  logic [1:0] abys_transformer_tmp3;
  logic abys_transformer_tmp4;
  logic [7:0] abys_transformer_tmp5;
  logic [1:0] abys_transformer_tmp6;
  logic abys_transformer_tmp7;
  logic [7:0] abys_transformer_tmp8;
  logic [1:0] abys_transformer_tmp9;
  logic abys_transformer_tmp10;
  logic [7:0] abys_transformer_tmp11;


  always @(*)   begin
    abys_transformer_tmp9 = write_index;
    abys_transformer_tmp10 = write_enable;
    abys_transformer_tmp11 = write_data;
  end
  always @(*)   begin
    logic abys_dumper_tmp4;
    abys_dumper_tmp4 = (!write_enable);
    abys_transformer_tmp6 = write_index;
    abys_transformer_tmp7 = abys_dumper_tmp4;
    abys_transformer_tmp8 = write_data;
  end
  always @(*)   begin
    read_data_if = abys_transformer_tmp2;
    abys_transformer_tmp0 = read_index;
    abys_transformer_tmp1 = 1'b1;
  end
  always @(*)   begin
    read_data_else = abys_transformer_tmp5;
    abys_transformer_tmp3 = read_index;
    abys_transformer_tmp4 = 1'b1;
  end
  always @(*) begin
    abys_transformer_tmp2 = memory_if[abys_transformer_tmp0];
  end
  always @(*) begin
    abys_transformer_tmp5 = memory_else[abys_transformer_tmp3];
  end
  always @(posedge clk) begin
    if (abys_transformer_tmp7) begin
      memory_else[abys_transformer_tmp6] <= abys_transformer_tmp8;
    end
  end
  always @(posedge clk) begin
    if (abys_transformer_tmp10) begin
      memory_if[abys_transformer_tmp9] <= abys_transformer_tmp11;
    end
  end
endmodule
