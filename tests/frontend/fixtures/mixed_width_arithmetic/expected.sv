module top (
  input signed [7:0] a,
  input [3:0] b,
  output  logic signed [8:0] y);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp3;
    logic [8:0] abys_dumper_tmp4;
    logic [8:0] abys_dumper_tmp6;
    logic [8:0] abys_dumper_tmp7;
    logic signed [8:0] abys_dumper_tmp8;
    abys_dumper_tmp3 = a;
    abys_dumper_tmp4 = abys_dumper_tmp3;
    abys_dumper_tmp6 = b;
    abys_dumper_tmp7 = (abys_dumper_tmp4 + abys_dumper_tmp6);
    abys_dumper_tmp8 = abys_dumper_tmp7;
    y = abys_dumper_tmp8;
  end
endmodule
