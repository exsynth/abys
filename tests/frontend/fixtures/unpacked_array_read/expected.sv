module top (
  input [7:0] a [0:3],
  input [1:0] index,
  output  logic [7:0] y);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp4;
    abys_dumper_tmp4 = a[index];
    y = abys_dumper_tmp4;
  end
endmodule
