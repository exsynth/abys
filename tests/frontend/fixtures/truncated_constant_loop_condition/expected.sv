module top (
  output  logic [15:0] y_o);



  always @(*)   begin
    logic [31:0] abys_dumper_tmp20;
    logic [15:0] abys_dumper_tmp21;
    logic [31:0] abys_dumper_tmp38;
    logic [15:0] abys_dumper_tmp39;
    logic [31:0] abys_dumper_tmp56;
    logic [15:0] abys_dumper_tmp57;
    logic [31:0] abys_dumper_tmp74;
    logic [15:0] abys_dumper_tmp75;
    logic [31:0] abys_dumper_tmp92;
    logic [15:0] abys_dumper_tmp93;
    logic [31:0] abys_dumper_tmp110;
    logic [15:0] abys_dumper_tmp111;
    logic [31:0] abys_dumper_tmp128;
    logic [15:0] abys_dumper_tmp129;
    logic [31:0] abys_dumper_tmp146;
    logic [15:0] abys_dumper_tmp147;
    abys_dumper_tmp20 = (1'b0 + 32'b1);
    abys_dumper_tmp21 = 16'b0;
    abys_dumper_tmp21[abys_dumper_tmp20] = 1'b1;
    abys_dumper_tmp38 = (1'b0 + 32'b11);
    abys_dumper_tmp39 = abys_dumper_tmp21;
    abys_dumper_tmp39[abys_dumper_tmp38] = 1'b1;
    abys_dumper_tmp56 = (1'b0 + 32'b101);
    abys_dumper_tmp57 = abys_dumper_tmp39;
    abys_dumper_tmp57[abys_dumper_tmp56] = 1'b1;
    abys_dumper_tmp74 = (1'b0 + 32'b111);
    abys_dumper_tmp75 = abys_dumper_tmp57;
    abys_dumper_tmp75[abys_dumper_tmp74] = 1'b1;
    abys_dumper_tmp92 = (1'b0 + 32'b1001);
    abys_dumper_tmp93 = abys_dumper_tmp75;
    abys_dumper_tmp93[abys_dumper_tmp92] = 1'b1;
    abys_dumper_tmp110 = (1'b0 + 32'b1011);
    abys_dumper_tmp111 = abys_dumper_tmp93;
    abys_dumper_tmp111[abys_dumper_tmp110] = 1'b1;
    abys_dumper_tmp128 = (1'b0 + 32'b1101);
    abys_dumper_tmp129 = abys_dumper_tmp111;
    abys_dumper_tmp129[abys_dumper_tmp128] = 1'b1;
    abys_dumper_tmp146 = (1'b0 + 32'b1111);
    abys_dumper_tmp147 = abys_dumper_tmp129;
    abys_dumper_tmp147[abys_dumper_tmp146] = 1'b1;
    y_o = abys_dumper_tmp147;
  end
endmodule
