module top (
  input [7:0] a,
  input [7:0] b,
  output  logic [7:0] sum,
  output  logic [7:0] difference);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp5;
    logic [7:0] abys_dumper_tmp4;
    abys_dumper_tmp5 = (a - b);
    abys_dumper_tmp4 = (a + b);
    difference = abys_dumper_tmp5;
    sum = abys_dumper_tmp4;
  end
endmodule
