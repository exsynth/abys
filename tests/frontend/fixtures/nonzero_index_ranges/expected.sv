module top (
  input [7:0] negative_range,
  input signed [3:0] negative_index,
  input [7:0] offset_range,
  input [9:0] offset_index,
  output  logic negative_y,
  output  logic offset_y);



  always @(*)   begin
    logic abys_dumper_tmp7;
    logic signed [4:0] abys_dumper_tmp4;
    logic signed [4:0] abys_dumper_tmp6;
    abys_dumper_tmp4 = negative_index;
    abys_dumper_tmp6 = (5'sb11 - abys_dumper_tmp4);
    abys_dumper_tmp7 = negative_range[abys_dumper_tmp6];
    negative_y = abys_dumper_tmp7;
  end
  always @(*)   begin
    logic abys_dumper_tmp7;
    logic signed [11:0] abys_dumper_tmp4;
    logic signed [11:0] abys_dumper_tmp6;
    abys_dumper_tmp4 = offset_index;
    abys_dumper_tmp6 = (abys_dumper_tmp4 - 12'sb1111100001);
    abys_dumper_tmp7 = offset_range[abys_dumper_tmp6];
    offset_y = abys_dumper_tmp7;
  end
endmodule
