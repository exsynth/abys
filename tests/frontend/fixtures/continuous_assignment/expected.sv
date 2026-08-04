module top (
  input [7:0] a,
  input [7:0] b,
  output  logic [7:0] y);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp4;
    abys_dumper_tmp4 = (a + b);
    y = abys_dumper_tmp4;
  end
endmodule
