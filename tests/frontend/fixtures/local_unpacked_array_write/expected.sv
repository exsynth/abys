module top (
  input [1:0] index,
  input [7:0] value,
  output  logic [7:0] y);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp27 [0:3];
    logic [7:0] abys_dumper_tmp29;
    abys_dumper_tmp27[0] = 8'bxxxxxxxx;
    abys_dumper_tmp27[1] = 8'bxxxxxxxx;
    abys_dumper_tmp27[2] = 8'bxxxxxxxx;
    abys_dumper_tmp27[3] = 8'bxxxxxxxx;
    abys_dumper_tmp27[32'sb0] = 8'b10001;
    abys_dumper_tmp27[32'sb1] = 8'b100010;
    abys_dumper_tmp27[32'sb10] = 8'b110011;
    abys_dumper_tmp27[32'sb11] = 8'b1000100;
    abys_dumper_tmp27[index] = value;
    abys_dumper_tmp29 = abys_dumper_tmp27[32'sb10];
    y = abys_dumper_tmp29;
  end
endmodule
