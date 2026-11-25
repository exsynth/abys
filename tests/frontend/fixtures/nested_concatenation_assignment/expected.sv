module top (
  input [7:0] a,
  output  logic [1:0] first,
  output  logic [3:0] middle,
  output  logic [1:0] last);



  always @(*)   begin
    logic [1:0] abys_dumper_tmp5;
    logic [3:0] abys_dumper_tmp4;
    logic [1:0] abys_dumper_tmp3;
    abys_dumper_tmp5 = a[1'b0 +: 2];
    abys_dumper_tmp4 = a[2'b10 +: 4];
    abys_dumper_tmp3 = a[3'b110 +: 2];
    last = abys_dumper_tmp5;
    middle = abys_dumper_tmp4;
    first = abys_dumper_tmp3;
  end
endmodule
