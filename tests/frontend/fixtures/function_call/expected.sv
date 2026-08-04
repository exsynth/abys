module top (
  input [7:0] a,
  output  logic [7:0] y);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp5;
    logic [7:0] abys_dumper_tmp3;
    abys_dumper_tmp5 = (~a);
    abys_dumper_tmp3 = abys_dumper_tmp5;
    y = abys_dumper_tmp3;
  end
endmodule
