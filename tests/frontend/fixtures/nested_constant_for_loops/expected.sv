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
    logic abys_dumper_tmp39;
    logic [1:0] abys_dumper_tmp38;
    logic [3:0] abys_dumper_tmp41;
    logic abys_dumper_tmp49;
    logic [1:0] abys_dumper_tmp48;
    logic [3:0] abys_dumper_tmp51;
    abys_dumper_tmp11 = a[1'b0 +: 2];
    abys_dumper_tmp12 = ((abys_dumper_tmp11 >> (1'b0)) & {1{1'b1}});
    abys_dumper_tmp14 = y;
    abys_dumper_tmp14[1'b0] = abys_dumper_tmp12;
    abys_dumper_tmp20 = a[1'b0 +: 2];
    abys_dumper_tmp21 = ((abys_dumper_tmp20 >> (1'b1)) & {1{1'b1}});
    abys_dumper_tmp22 = abys_dumper_tmp14;
    abys_dumper_tmp22[1'b1] = abys_dumper_tmp21;
    abys_dumper_tmp38 = a[2'b10 +: 2];
    abys_dumper_tmp39 = ((abys_dumper_tmp38 >> (1'b0)) & {1{1'b1}});
    abys_dumper_tmp41 = abys_dumper_tmp22;
    abys_dumper_tmp41[2'b10] = abys_dumper_tmp39;
    abys_dumper_tmp48 = a[2'b10 +: 2];
    abys_dumper_tmp49 = ((abys_dumper_tmp48 >> (1'b1)) & {1{1'b1}});
    abys_dumper_tmp51 = abys_dumper_tmp41;
    abys_dumper_tmp51[2'b11] = abys_dumper_tmp49;
    y = abys_dumper_tmp51;
  end
endmodule
