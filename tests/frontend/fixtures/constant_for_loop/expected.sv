module top (
  input [3:0] a,
  input [3:0] b,
  output  logic [3:0] y);



  always @(*)   begin
    logic abys_dumper_tmp7;
    logic abys_dumper_tmp9;
    logic abys_dumper_tmp10;
    logic [31:0] abys_dumper_tmp11;
    logic [3:0] abys_dumper_tmp13;
    logic abys_dumper_tmp19;
    logic abys_dumper_tmp20;
    logic abys_dumper_tmp21;
    logic [31:0] abys_dumper_tmp22;
    logic [3:0] abys_dumper_tmp23;
    logic abys_dumper_tmp29;
    logic abys_dumper_tmp30;
    logic abys_dumper_tmp31;
    logic [31:0] abys_dumper_tmp32;
    logic [3:0] abys_dumper_tmp33;
    logic abys_dumper_tmp39;
    logic abys_dumper_tmp40;
    logic abys_dumper_tmp41;
    logic [31:0] abys_dumper_tmp42;
    logic [3:0] abys_dumper_tmp43;
    abys_dumper_tmp7 = a[32'sb0];
    abys_dumper_tmp9 = b[32'sb0];
    abys_dumper_tmp10 = (abys_dumper_tmp7 & abys_dumper_tmp9);
    abys_dumper_tmp11 = (1'b0 + 32'sb0);
    abys_dumper_tmp13 = y;
    abys_dumper_tmp13[abys_dumper_tmp11] = abys_dumper_tmp10;
    abys_dumper_tmp19 = a[32'sb1];
    abys_dumper_tmp20 = b[32'sb1];
    abys_dumper_tmp21 = (abys_dumper_tmp19 & abys_dumper_tmp20);
    abys_dumper_tmp22 = (1'b0 + 32'sb1);
    abys_dumper_tmp23 = abys_dumper_tmp13;
    abys_dumper_tmp23[abys_dumper_tmp22] = abys_dumper_tmp21;
    abys_dumper_tmp29 = a[32'sb10];
    abys_dumper_tmp30 = b[32'sb10];
    abys_dumper_tmp31 = (abys_dumper_tmp29 & abys_dumper_tmp30);
    abys_dumper_tmp32 = (1'b0 + 32'sb10);
    abys_dumper_tmp33 = abys_dumper_tmp23;
    abys_dumper_tmp33[abys_dumper_tmp32] = abys_dumper_tmp31;
    abys_dumper_tmp39 = a[32'sb11];
    abys_dumper_tmp40 = b[32'sb11];
    abys_dumper_tmp41 = (abys_dumper_tmp39 & abys_dumper_tmp40);
    abys_dumper_tmp42 = (1'b0 + 32'sb11);
    abys_dumper_tmp43 = abys_dumper_tmp33;
    abys_dumper_tmp43[abys_dumper_tmp42] = abys_dumper_tmp41;
    y = abys_dumper_tmp43;
  end
endmodule
