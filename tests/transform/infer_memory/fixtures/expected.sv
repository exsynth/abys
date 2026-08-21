module top (
  input clk,
  input [2:0] write_index,
  input [2:0] read_base,
  input offset,
  input [7:0] write_data,
  output  logic [7:0] read_data);

  logic [7:0] memory [0:7];
  logic signed [3:0] abys_transformer_tmp0;
  logic abys_transformer_tmp1;
  logic [7:0] abys_transformer_tmp2 [0:1];
  logic [2:0] abys_transformer_tmp3;
  logic abys_transformer_tmp4;
  logic [7:0] abys_transformer_tmp5;
  logic abys_transformer_tmp6;
  logic [3:0] abys_transformer_tmp7;
  logic [7:0] abys_transformer_tmp8;


  always @(*)   begin
    abys_transformer_tmp3 = write_index;
    abys_transformer_tmp4 = 1'b1;
    abys_transformer_tmp5 = write_data;
  end
  always @(*)   begin
    logic [7:0] abys_dumper_tmp3;
    logic signed [3:0] abys_dumper_tmp5;
    logic [3:0] abys_dumper_tmp8;
    abys_dumper_tmp3 = abys_transformer_tmp8;
    abys_dumper_tmp5 = read_base;
    abys_dumper_tmp8 = (abys_dumper_tmp5 + offset);
    read_data = abys_dumper_tmp3;
    abys_transformer_tmp0 = abys_dumper_tmp5;
    abys_transformer_tmp1 = 1'b1;
    abys_transformer_tmp6 = offset;
    abys_transformer_tmp7 = abys_dumper_tmp8;
  end
  always @(*) begin
    abys_transformer_tmp2 = memory[abys_transformer_tmp0 +: 2];
  end
  always @(*) begin
    abys_transformer_tmp8 = memory[abys_transformer_tmp7];
  end
  always @(posedge clk) begin
    if (abys_transformer_tmp4) begin
      memory[abys_transformer_tmp3] <= abys_transformer_tmp5;
    end
  end
endmodule
