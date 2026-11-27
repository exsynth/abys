module top (
  input [7:0] a,
  input [3:0] upper,
  output  logic [7:0] y);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp4;
    abys_dumper_tmp4 = a;
    abys_dumper_tmp4[3'b100 +: 3'b100] = upper;
    y = abys_dumper_tmp4;
  end
endmodule
