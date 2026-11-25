module top (
  input [15:0] a,
  output  logic [7:0] y);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp3;
    abys_dumper_tmp3 = a[3'b100 +: 8];
    y = abys_dumper_tmp3;
  end
endmodule
