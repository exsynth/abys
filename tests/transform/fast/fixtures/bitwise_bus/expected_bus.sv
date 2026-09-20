module top (
  input [3:0] a,
  input [3:0] b,
  input [3:0] c,
  input [3:0] d,
  output  logic [3:0] y);



  always @(*)   begin
    logic [3:0] abys_dumper_tmp4;
    logic [3:0] abys_dumper_tmp7;
    logic [3:0] abys_dumper_tmp8;
    abys_dumper_tmp4 = (b & a);
    abys_dumper_tmp7 = (d & c);
    abys_dumper_tmp8 = (abys_dumper_tmp4 & abys_dumper_tmp7);
    y = abys_dumper_tmp8;
  end
endmodule
