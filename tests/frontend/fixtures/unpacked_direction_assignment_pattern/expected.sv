module top (
  input [7:0] value3,
  input [7:0] value2,
  input [7:0] value1,
  input [7:0] value0,
  output  logic [7:0] result3,
  output  logic [7:0] result2,
  output  logic [7:0] result1,
  output  logic [7:0] result0);

  logic [7:0] values [0:3];


  always @(*)   begin
    logic [7:0] abys_dumper_tmp8 [0:3];
    logic [7:0] abys_dumper_tmp16;
    logic [7:0] abys_dumper_tmp12;
    logic [7:0] abys_dumper_tmp10;
    logic [7:0] abys_dumper_tmp14;
    abys_dumper_tmp8 = values;
    abys_dumper_tmp8[0] = value0;
    abys_dumper_tmp8[1] = value1;
    abys_dumper_tmp8[2] = value2;
    abys_dumper_tmp8[3] = value3;
    abys_dumper_tmp16 = abys_dumper_tmp8[32'sb0];
    abys_dumper_tmp12 = abys_dumper_tmp8[32'sb10];
    abys_dumper_tmp10 = abys_dumper_tmp8[32'sb11];
    abys_dumper_tmp14 = abys_dumper_tmp8[32'sb1];
    result0 = abys_dumper_tmp16;
    result2 = abys_dumper_tmp12;
    result3 = abys_dumper_tmp10;
    result1 = abys_dumper_tmp14;
    values[0] = value0;
    values[1] = value1;
    values[2] = value2;
    values[3] = value3;
  end
endmodule
