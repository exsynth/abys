module top (
  input [3:0] a,
  output  logic [7:0] y);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp3;
    abys_dumper_tmp3 = a;
    y = abys_dumper_tmp3;
  end
endmodule
