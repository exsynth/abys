module top (
  input [7:0] a,
  output  logic [7:0] y);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp3;
    logic [7:0] abys_dumper_tmp4;
    abys_dumper_tmp3 = (~a);
    abys_dumper_tmp4 = abys_dumper_tmp3;
    y = abys_dumper_tmp4;
  end
endmodule
