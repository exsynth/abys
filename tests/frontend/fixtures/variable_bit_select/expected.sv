module top (
  input [15:0] a,
  input [3:0] index,
  output  logic y);



  always @(*)   begin
    logic abys_dumper_tmp5;
    abys_dumper_tmp5 = a[($signed(9'({1'b0, 1'b0})) + ($signed(9'({1'b0, index})) * $signed(9'($signed(2'sb1)))))];
    y = abys_dumper_tmp5;
  end
endmodule
