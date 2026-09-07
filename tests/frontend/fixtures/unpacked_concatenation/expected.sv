module top (
  input [7:0] values [0:3],
  output  logic [7:0] concatenated [0:3]);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp3 [0:0];
    logic [7:0] abys_dumper_tmp4 [0:2];
    logic [7:0] abys_dumper_tmp5 [0:3];
    abys_dumper_tmp3 = values[1'b0 +: 1];
    abys_dumper_tmp4 = values[1'b1 +: 3];
    abys_dumper_tmp5 = {abys_dumper_tmp3, abys_dumper_tmp4};
    concatenated = abys_dumper_tmp5;
  end
endmodule
