module top (
  input [1:0] index,
  input [7:0] value,
  output  logic [7:0] y);

  logic [7:0] memory [0:3];


  always @(*)   begin
    logic [7:0] abys_dumper_tmp22 [0:3];
    logic [7:0] abys_dumper_tmp24;
    abys_dumper_tmp22 = memory;
    abys_dumper_tmp22[32'sb0] = 8'b10001;
    abys_dumper_tmp22[32'sb1] = 8'b100010;
    abys_dumper_tmp22[32'sb10] = 8'b110011;
    abys_dumper_tmp22[32'sb11] = 8'b1000100;
    abys_dumper_tmp22[index] = value;
    abys_dumper_tmp24 = abys_dumper_tmp22[32'sb10];
    y = abys_dumper_tmp24;
    memory[32'sb0] = 8'b10001;
    memory[32'sb1] = 8'b100010;
    memory[32'sb10] = 8'b110011;
    memory[32'sb11] = 8'b1000100;
    memory[index] = value;
  end
endmodule
