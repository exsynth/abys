module top (
  input clk,
  input [2:0] write_index,
  input [2:0] read_base,
  input [7:0] write_data,
  output  logic [7:0] read_data);

  logic [7:0] memory [7:0];
  logic signed [4:0] abys_transformer_tmp0;
  logic abys_transformer_tmp1;
  logic [7:0] abys_transformer_tmp2 [0:0];
  logic signed [4:0] abys_transformer_tmp3;
  logic abys_transformer_tmp4;
  logic [7:0] abys_transformer_tmp5;
  logic signed [31:0] abys_transformer_tmp6;
  logic signed [31:0] abys_transformer_tmp7;
  logic [7:0] abys_transformer_tmp8;


  always @(*)   begin
    logic signed [4:0] abys_dumper_tmp4;
    logic signed [4:0] abys_dumper_tmp5;
    abys_dumper_tmp4 = write_index;
    abys_dumper_tmp5 = (5'sb111 - abys_dumper_tmp4);
    abys_transformer_tmp3 = abys_dumper_tmp5;
    abys_transformer_tmp4 = 1'b1;
    abys_transformer_tmp5 = write_data;
  end
  always @(*)   begin
    logic [7:0] abys_dumper_tmp3;
    logic signed [4:0] abys_dumper_tmp6;
    logic signed [4:0] abys_dumper_tmp7;
    logic signed [31:0] abys_dumper_tmp9;
    abys_dumper_tmp3 = abys_transformer_tmp8;
    abys_dumper_tmp6 = read_base;
    abys_dumper_tmp7 = (5'sb111 - abys_dumper_tmp6);
    abys_dumper_tmp9 = (abys_dumper_tmp7 + 32'sb0);
    read_data = abys_dumper_tmp3;
    abys_transformer_tmp0 = abys_dumper_tmp7;
    abys_transformer_tmp1 = 1'b1;
    abys_transformer_tmp6 = 32'sb0;
    abys_transformer_tmp7 = abys_dumper_tmp9;
  end
  always @(*) begin
    abys_transformer_tmp2 = memory[abys_transformer_tmp0 +: 1];
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
