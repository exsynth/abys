module top (
  input [7:0] a,
  input [7:0] b,
  input [7:0] c,
  output  logic [7:0] y);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp4;
    logic [7:0] abys_dumper_tmp6;
    abys_dumper_tmp4 = (a & b);
    abys_dumper_tmp6 = (abys_dumper_tmp4 | c);
    y = abys_dumper_tmp6;
  end
endmodule
