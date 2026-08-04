module top (
  input [7:0] a [3:0],
  input [1:0] index,
  output  logic [7:0] y);



  always @(*)   begin
    logic signed [3:0] abys_dumper_tmp4;
    logic signed [3:0] abys_dumper_tmp6;
    logic [7:0] abys_dumper_tmp7;
    abys_dumper_tmp4 = index;
    abys_dumper_tmp6 = (4'sb11 - abys_dumper_tmp4);
    abys_dumper_tmp7 = a[abys_dumper_tmp6];
    y = abys_dumper_tmp7;
  end
endmodule
