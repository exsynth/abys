module top (
  input [7:0] negative_range,
  input signed [3:0] negative_index,
  input [7:0] offset_range,
  input [9:0] offset_index,
  output  logic negative_y,
  output  logic offset_y);



  always @(*)   begin
    logic abys_dumper_tmp6;
    abys_dumper_tmp6 = negative_range[($signed(8'({1'b0, 2'b11})) + ($signed(8'($signed(negative_index))) * $signed(8'($signed(-2'sb1)))))];
    negative_y = abys_dumper_tmp6;
  end
  always @(*)   begin
    logic abys_dumper_tmp6;
    abys_dumper_tmp6 = offset_range[($signed(15'($signed(-11'sb1111100001))) + ($signed(15'({1'b0, offset_index})) * $signed(15'($signed(2'sb1)))))];
    offset_y = abys_dumper_tmp6;
  end
endmodule
