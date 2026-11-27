module top (
  input [3:0] a,
  input [3:0] b,
  output  logic [3:0] y);



  always @(*)   begin
    logic abys_dumper_tmp7;
    logic abys_dumper_tmp9;
    logic abys_dumper_tmp10;
    logic [3:0] abys_dumper_tmp12;
    logic abys_dumper_tmp18;
    logic abys_dumper_tmp19;
    logic abys_dumper_tmp20;
    logic [3:0] abys_dumper_tmp21;
    logic abys_dumper_tmp27;
    logic abys_dumper_tmp28;
    logic abys_dumper_tmp29;
    logic [3:0] abys_dumper_tmp30;
    logic abys_dumper_tmp36;
    logic abys_dumper_tmp37;
    logic abys_dumper_tmp38;
    logic [3:0] abys_dumper_tmp39;
    abys_dumper_tmp7 = a[1'b0];
    abys_dumper_tmp9 = b[1'b0];
    abys_dumper_tmp10 = (abys_dumper_tmp7 & abys_dumper_tmp9);
    abys_dumper_tmp12 = y;
    abys_dumper_tmp12[1'b0] = abys_dumper_tmp10;
    abys_dumper_tmp18 = a[1'b1];
    abys_dumper_tmp19 = b[1'b1];
    abys_dumper_tmp20 = (abys_dumper_tmp18 & abys_dumper_tmp19);
    abys_dumper_tmp21 = abys_dumper_tmp12;
    abys_dumper_tmp21[1'b1] = abys_dumper_tmp20;
    abys_dumper_tmp27 = a[2'b10];
    abys_dumper_tmp28 = b[2'b10];
    abys_dumper_tmp29 = (abys_dumper_tmp27 & abys_dumper_tmp28);
    abys_dumper_tmp30 = abys_dumper_tmp21;
    abys_dumper_tmp30[2'b10] = abys_dumper_tmp29;
    abys_dumper_tmp36 = a[2'b11];
    abys_dumper_tmp37 = b[2'b11];
    abys_dumper_tmp38 = (abys_dumper_tmp36 & abys_dumper_tmp37);
    abys_dumper_tmp39 = abys_dumper_tmp30;
    abys_dumper_tmp39[2'b11] = abys_dumper_tmp38;
    y = abys_dumper_tmp39;
  end
endmodule
