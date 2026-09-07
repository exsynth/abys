module top (
  output  logic [15:0] y_o);



  always @(*)   begin
    logic [15:0] abys_dumper_tmp20;
    logic [15:0] abys_dumper_tmp38;
    logic [15:0] abys_dumper_tmp56;
    logic [15:0] abys_dumper_tmp74;
    logic [15:0] abys_dumper_tmp92;
    logic [15:0] abys_dumper_tmp110;
    logic [15:0] abys_dumper_tmp128;
    logic [15:0] abys_dumper_tmp146;
    abys_dumper_tmp20 = 16'b0;
    abys_dumper_tmp20[1'b1] = 1'b1;
    abys_dumper_tmp38 = abys_dumper_tmp20;
    abys_dumper_tmp38[2'b11] = 1'b1;
    abys_dumper_tmp56 = abys_dumper_tmp38;
    abys_dumper_tmp56[3'b101] = 1'b1;
    abys_dumper_tmp74 = abys_dumper_tmp56;
    abys_dumper_tmp74[3'b111] = 1'b1;
    abys_dumper_tmp92 = abys_dumper_tmp74;
    abys_dumper_tmp92[4'b1001] = 1'b1;
    abys_dumper_tmp110 = abys_dumper_tmp92;
    abys_dumper_tmp110[4'b1011] = 1'b1;
    abys_dumper_tmp128 = abys_dumper_tmp110;
    abys_dumper_tmp128[4'b1101] = 1'b1;
    abys_dumper_tmp146 = abys_dumper_tmp128;
    abys_dumper_tmp146[4'b1111] = 1'b1;
    y_o = abys_dumper_tmp146;
  end
endmodule
