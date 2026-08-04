module top (
  input [3:0] a,
  output  logic [3:0] y);



  always @(*)   begin
    logic abys_dumper_tmp14;
    logic [1:0] abys_dumper_tmp13;
    logic [31:0] abys_dumper_tmp12;
    logic [31:0] abys_dumper_tmp15;
    logic [31:0] abys_dumper_tmp17;
    logic [31:0] abys_dumper_tmp18;
    logic [3:0] abys_dumper_tmp20;
    logic abys_dumper_tmp29;
    logic [1:0] abys_dumper_tmp28;
    logic [31:0] abys_dumper_tmp27;
    logic [31:0] abys_dumper_tmp30;
    logic [31:0] abys_dumper_tmp32;
    logic [31:0] abys_dumper_tmp33;
    logic [3:0] abys_dumper_tmp34;
    logic abys_dumper_tmp52;
    logic [1:0] abys_dumper_tmp51;
    logic [31:0] abys_dumper_tmp50;
    logic [31:0] abys_dumper_tmp53;
    logic [31:0] abys_dumper_tmp55;
    logic [31:0] abys_dumper_tmp56;
    logic [3:0] abys_dumper_tmp57;
    logic abys_dumper_tmp66;
    logic [1:0] abys_dumper_tmp65;
    logic [31:0] abys_dumper_tmp64;
    logic [31:0] abys_dumper_tmp67;
    logic [31:0] abys_dumper_tmp69;
    logic [31:0] abys_dumper_tmp70;
    logic [3:0] abys_dumper_tmp71;
    abys_dumper_tmp12 = (32'sb0 * 4'd2);
    abys_dumper_tmp13 = a[abys_dumper_tmp12 +: 2];
    abys_dumper_tmp14 = ((abys_dumper_tmp13 >> (32'sb0)) & {1{1'b1}});
    abys_dumper_tmp15 = (1'b0 + 32'sb0);
    abys_dumper_tmp17 = (32'sb0 * 4'd2);
    abys_dumper_tmp18 = (abys_dumper_tmp15 + abys_dumper_tmp17);
    abys_dumper_tmp20 = y;
    abys_dumper_tmp20[abys_dumper_tmp18] = abys_dumper_tmp14;
    abys_dumper_tmp27 = (32'sb0 * 4'd2);
    abys_dumper_tmp28 = a[abys_dumper_tmp27 +: 2];
    abys_dumper_tmp29 = ((abys_dumper_tmp28 >> (32'sb1)) & {1{1'b1}});
    abys_dumper_tmp30 = (1'b0 + 32'sb1);
    abys_dumper_tmp32 = (32'sb0 * 4'd2);
    abys_dumper_tmp33 = (abys_dumper_tmp30 + abys_dumper_tmp32);
    abys_dumper_tmp34 = abys_dumper_tmp20;
    abys_dumper_tmp34[abys_dumper_tmp33] = abys_dumper_tmp29;
    abys_dumper_tmp50 = (32'sb1 * 4'd2);
    abys_dumper_tmp51 = a[abys_dumper_tmp50 +: 2];
    abys_dumper_tmp52 = ((abys_dumper_tmp51 >> (32'sb0)) & {1{1'b1}});
    abys_dumper_tmp53 = (1'b0 + 32'sb0);
    abys_dumper_tmp55 = (32'sb1 * 4'd2);
    abys_dumper_tmp56 = (abys_dumper_tmp53 + abys_dumper_tmp55);
    abys_dumper_tmp57 = abys_dumper_tmp34;
    abys_dumper_tmp57[abys_dumper_tmp56] = abys_dumper_tmp52;
    abys_dumper_tmp64 = (32'sb1 * 4'd2);
    abys_dumper_tmp65 = a[abys_dumper_tmp64 +: 2];
    abys_dumper_tmp66 = ((abys_dumper_tmp65 >> (32'sb1)) & {1{1'b1}});
    abys_dumper_tmp67 = (1'b0 + 32'sb1);
    abys_dumper_tmp69 = (32'sb1 * 4'd2);
    abys_dumper_tmp70 = (abys_dumper_tmp67 + abys_dumper_tmp69);
    abys_dumper_tmp71 = abys_dumper_tmp57;
    abys_dumper_tmp71[abys_dumper_tmp70] = abys_dumper_tmp66;
    y = abys_dumper_tmp71;
  end
endmodule
