module top (
  input [3:0] a,
  output  logic [3:0] y);



  always @(*)   begin
    logic abys_dumper_tmp12;
    logic [1:0] abys_dumper_tmp11;
    logic [3:0] abys_dumper_tmp14;
    logic abys_dumper_tmp21;
    logic [1:0] abys_dumper_tmp20;
    logic [3:0] abys_dumper_tmp22;
    logic abys_dumper_tmp38;
    logic [1:0] abys_dumper_tmp37;
    logic [3:0] abys_dumper_tmp39;
    logic abys_dumper_tmp46;
    logic [1:0] abys_dumper_tmp45;
    logic [3:0] abys_dumper_tmp47;
    abys_dumper_tmp11 = a[1'b0 +: 2];
    abys_dumper_tmp12 = ((abys_dumper_tmp11 >> (1'b0)) & {1{1'b1}});
    abys_dumper_tmp14 = y;
    abys_dumper_tmp14[1'b0] = abys_dumper_tmp12;
    abys_dumper_tmp20 = a[1'b0 +: 2];
    abys_dumper_tmp21 = ((abys_dumper_tmp20 >> (1'b1)) & {1{1'b1}});
    abys_dumper_tmp22 = abys_dumper_tmp14;
    abys_dumper_tmp22[1'b1] = abys_dumper_tmp21;
    abys_dumper_tmp37 = a[2'b10 +: 2];
    abys_dumper_tmp38 = ((abys_dumper_tmp37 >> (1'b0)) & {1{1'b1}});
    abys_dumper_tmp39 = abys_dumper_tmp22;
    abys_dumper_tmp39[2'b10] = abys_dumper_tmp38;
    abys_dumper_tmp45 = a[2'b10 +: 2];
    abys_dumper_tmp46 = ((abys_dumper_tmp45 >> (1'b1)) & {1{1'b1}});
    abys_dumper_tmp47 = abys_dumper_tmp39;
    abys_dumper_tmp47[2'b11] = abys_dumper_tmp46;
    y = abys_dumper_tmp47;
  end
endmodule
