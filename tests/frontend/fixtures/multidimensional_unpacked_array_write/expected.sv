module top (
  input row,
  input column,
  input [7:0] value,
  output  logic [7:0] y);

  logic [7:0] memory [0:1] [0:1];


  always @(*)   begin
    logic [7:0] abys_dumper_tmp32 [0:1] [0:1];
    logic [7:0] abys_dumper_tmp34 [0:1];
    logic [7:0] abys_dumper_tmp36;
    abys_dumper_tmp32 = memory;
    abys_dumper_tmp32[32'sb0][32'sb0] = 8'b10001;
    abys_dumper_tmp32[32'sb0][32'sb1] = 8'b100010;
    abys_dumper_tmp32[32'sb1][32'sb0] = 8'b110011;
    abys_dumper_tmp32[32'sb1][32'sb1] = 8'b1000100;
    abys_dumper_tmp32[row][column] = value;
    abys_dumper_tmp34 = abys_dumper_tmp32[32'sb1];
    abys_dumper_tmp36 = abys_dumper_tmp34[32'sb0];
    y = abys_dumper_tmp36;
    memory[32'sb0][32'sb0] = 8'b10001;
    memory[32'sb0][32'sb1] = 8'b100010;
    memory[32'sb1][32'sb0] = 8'b110011;
    memory[32'sb1][32'sb1] = 8'b1000100;
    memory[row][column] = value;
  end
endmodule
