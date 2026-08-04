module top (
  input [7:0] a,
  output  logic [3:0] upper,
  output  logic [3:0] lower);



  always @(*)   begin
    logic [3:0] abys_dumper_tmp5;
    logic [3:0] abys_dumper_tmp4;
    abys_dumper_tmp5 = a[1'b0 +: 4];
    abys_dumper_tmp4 = a[3'b100 +: 4];
    lower = abys_dumper_tmp5;
    upper = abys_dumper_tmp4;
  end
endmodule
