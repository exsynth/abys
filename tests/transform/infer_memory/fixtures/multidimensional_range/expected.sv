module top (
  input clk,
  input [1:0] write_row,
  input [1:0] write_column,
  input [1:0] read_row,
  input [1:0] read_base,
  input read_offset,
  input [7:0] write_data,
  output  logic [7:0] read_data);

  logic [7:0] memory [3:0] [3:0];
  logic [1:0] abys_transformer_tmp0;
  logic [1:0] abys_transformer_tmp1;
  logic abys_transformer_tmp2;
  logic [7:0] abys_transformer_tmp3 [1:0];
  logic [1:0] abys_transformer_tmp4;
  logic [1:0] abys_transformer_tmp5;
  logic abys_transformer_tmp6;
  logic [7:0] abys_transformer_tmp7;
  logic signed [2:0] abys_transformer_tmp8;
  logic [2:0] abys_transformer_tmp9;
  logic [7:0] abys_transformer_tmp10;


  always @(*)   begin
    abys_transformer_tmp4 = write_row;
    abys_transformer_tmp5 = write_column;
    abys_transformer_tmp6 = 1'b1;
    abys_transformer_tmp7 = write_data;
  end
  always @(*)   begin
    logic [7:0] abys_dumper_tmp3;
    logic signed [2:0] abys_dumper_tmp9;
    logic signed [2:0] abys_dumper_tmp10;
    logic [2:0] abys_dumper_tmp11;
    abys_dumper_tmp3 = abys_transformer_tmp10;
    abys_dumper_tmp9 = read_offset;
    abys_dumper_tmp10 = (3'sb1 - abys_dumper_tmp9);
    abys_dumper_tmp11 = (read_base + abys_dumper_tmp10);
    read_data = abys_dumper_tmp3;
    abys_transformer_tmp0 = read_row;
    abys_transformer_tmp1 = read_base;
    abys_transformer_tmp2 = 1'b1;
    abys_transformer_tmp8 = abys_dumper_tmp10;
    abys_transformer_tmp9 = abys_dumper_tmp11;
  end
  always @(*) begin
    abys_transformer_tmp3 = memory[abys_transformer_tmp0][abys_transformer_tmp1 +: 2];
  end
  always @(*) begin
    abys_transformer_tmp10 = memory[abys_transformer_tmp0][abys_transformer_tmp9];
  end
  always @(posedge clk) begin
    if (abys_transformer_tmp6) begin
      memory[abys_transformer_tmp4][abys_transformer_tmp5] <= abys_transformer_tmp7;
    end
  end
endmodule
