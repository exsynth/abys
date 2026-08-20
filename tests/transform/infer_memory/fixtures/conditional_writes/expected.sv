module top (
  input clk,
  input write_enable,
  input [1:0] write_index,
  input [1:0] read_index,
  input [7:0] write_data,
  output  logic [7:0] read_data);

  logic [7:0] memory [3:0];
  logic [1:0] abys_transformer_tmp0;
  logic abys_transformer_tmp1;
  logic [7:0] abys_transformer_tmp2;
  logic [1:0] abys_transformer_tmp3;
  logic abys_transformer_tmp4;
  logic [7:0] abys_transformer_tmp5;


  always @(*)   begin
    abys_transformer_tmp3 = write_index;
    abys_transformer_tmp4 = write_enable;
    abys_transformer_tmp5 = write_data;
  end
  always @(*)   begin
    read_data = abys_transformer_tmp2;
    abys_transformer_tmp0 = read_index;
    abys_transformer_tmp1 = 1'b1;
  end
  always @(*) begin
    abys_transformer_tmp2 = memory[abys_transformer_tmp0];
  end
  always @(posedge clk) begin
    begin
      if (abys_transformer_tmp4) begin
        memory[abys_transformer_tmp3] <= abys_transformer_tmp5;
      end else begin
      end
    end
  end
endmodule
