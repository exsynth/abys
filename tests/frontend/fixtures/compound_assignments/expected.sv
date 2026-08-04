module top (
  input [7:0] a,
  input [2:0] shift,
  output  logic [7:0] y);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp4;
    logic [7:0] abys_dumper_tmp6;
    logic [7:0] abys_dumper_tmp8;
    logic [7:0] abys_dumper_tmp10;
    logic [7:0] abys_dumper_tmp12;
    logic [7:0] abys_dumper_tmp14;
    logic [7:0] abys_dumper_tmp16;
    abys_dumper_tmp4 = (a + 8'b11);
    abys_dumper_tmp6 = (abys_dumper_tmp4 - 8'b1);
    abys_dumper_tmp8 = (abys_dumper_tmp6 & 8'b11111110);
    abys_dumper_tmp10 = (abys_dumper_tmp8 | 8'b10000);
    abys_dumper_tmp12 = (abys_dumper_tmp10 ^ 8'b1010101);
    abys_dumper_tmp14 = (abys_dumper_tmp12 << shift);
    abys_dumper_tmp16 = (abys_dumper_tmp14 >> 32'sb1);
    y = abys_dumper_tmp16;
  end
endmodule
