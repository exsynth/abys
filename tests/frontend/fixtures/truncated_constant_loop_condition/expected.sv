module top (
  output  logic [15:0] y_o);



  always @(*)   begin
    logic [15:0] abys_dumper_tmp20;
    logic [15:0] abys_dumper_tmp37;
    logic [15:0] abys_dumper_tmp54;
    logic [15:0] abys_dumper_tmp71;
    logic [15:0] abys_dumper_tmp88;
    logic [15:0] abys_dumper_tmp105;
    logic [15:0] abys_dumper_tmp122;
    logic [15:0] abys_dumper_tmp139;
    abys_dumper_tmp20 = 16'b0;
    abys_dumper_tmp20[1'b1] = 1'b1;
    abys_dumper_tmp37 = abys_dumper_tmp20;
    abys_dumper_tmp37[2'b11] = 1'b1;
    abys_dumper_tmp54 = abys_dumper_tmp37;
    abys_dumper_tmp54[3'b101] = 1'b1;
    abys_dumper_tmp71 = abys_dumper_tmp54;
    abys_dumper_tmp71[3'b111] = 1'b1;
    abys_dumper_tmp88 = abys_dumper_tmp71;
    abys_dumper_tmp88[4'b1001] = 1'b1;
    abys_dumper_tmp105 = abys_dumper_tmp88;
    abys_dumper_tmp105[4'b1011] = 1'b1;
    abys_dumper_tmp122 = abys_dumper_tmp105;
    abys_dumper_tmp122[4'b1101] = 1'b1;
    abys_dumper_tmp139 = abys_dumper_tmp122;
    abys_dumper_tmp139[4'b1111] = 1'b1;
    y_o = abys_dumper_tmp139;
  end
endmodule
