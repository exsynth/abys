module top (
  input [31:0] a,
  input [4:0] base,
  output  logic [7:0] y);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp4;
    abys_dumper_tmp4 = a[base +: 8];
    y = abys_dumper_tmp4;
  end
endmodule
