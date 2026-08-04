module top (
  input [31:0] a,
  input [4:0] base,
  output  logic [7:0] y);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp9;
    logic signed [7:0] abys_dumper_tmp4;
    logic signed [7:0] abys_dumper_tmp6;
    logic signed [7:0] abys_dumper_tmp8;
    abys_dumper_tmp4 = base;
    abys_dumper_tmp6 = (abys_dumper_tmp4 + -8'sb111);
    abys_dumper_tmp8 = (abys_dumper_tmp6 - 8'sb0);
    abys_dumper_tmp9 = a[abys_dumper_tmp8 +: 8];
    y = abys_dumper_tmp9;
  end
endmodule
