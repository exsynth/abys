module top (
  input [7:0] a,
  input [3:0] upper,
  output  logic [7:0] y);



  always @(*)   begin
    logic [2:0] abys_dumper_tmp5;
    logic [7:0] abys_dumper_tmp7;
    abys_dumper_tmp5 = (1'b0 + 3'b100);
    abys_dumper_tmp7 = a;
    abys_dumper_tmp7[abys_dumper_tmp5 +: 3'b100] = upper;
    y = abys_dumper_tmp7;
  end
endmodule
