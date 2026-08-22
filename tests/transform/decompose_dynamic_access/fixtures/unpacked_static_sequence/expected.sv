module top (
  input [7:0] values [0:3],
  input [7:0] update0,
  input [7:0] update1,
  input [7:0] update2,
  output  logic [7:0] updated [0:3]);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp4;
    logic [7:0] abys_dumper_tmp7;
    abys_dumper_tmp4 = values[1'b1];
    abys_dumper_tmp7 = values[2'b11];
    updated[0] = update1;
    updated[1] = abys_dumper_tmp4;
    updated[2] = update2;
    updated[3] = abys_dumper_tmp7;
  end
endmodule
