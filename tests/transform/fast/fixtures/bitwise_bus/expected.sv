module top (
  input [3:0] a,
  input [3:0] b,
  input [3:0] c,
  input [3:0] d,
  output  logic [3:0] y);



  always @(*)   begin
    logic abys_dumper_tmp4;
    logic abys_dumper_tmp7;
    logic abys_dumper_tmp8;
    logic abys_dumper_tmp11;
    logic abys_dumper_tmp14;
    logic abys_dumper_tmp15;
    logic abys_dumper_tmp16;
    logic abys_dumper_tmp18;
    logic abys_dumper_tmp20;
    logic abys_dumper_tmp21;
    logic abys_dumper_tmp23;
    logic abys_dumper_tmp25;
    logic abys_dumper_tmp26;
    logic abys_dumper_tmp27;
    logic abys_dumper_tmp28;
    logic abys_dumper_tmp29;
    logic abys_dumper_tmp30;
    logic abys_dumper_tmp31;
    logic abys_dumper_tmp32;
    logic abys_dumper_tmp33;
    logic abys_dumper_tmp34;
    logic abys_dumper_tmp35;
    logic abys_dumper_tmp36;
    logic abys_dumper_tmp37;
    logic abys_dumper_tmp38;
    logic abys_dumper_tmp39;
    logic abys_dumper_tmp40;
    logic abys_dumper_tmp41;
    logic [3:0] abys_dumper_tmp42;
    abys_dumper_tmp4 = b[2'b11];
    abys_dumper_tmp7 = a[2'b11];
    abys_dumper_tmp8 = (abys_dumper_tmp4 & abys_dumper_tmp7);
    abys_dumper_tmp11 = d[2'b11];
    abys_dumper_tmp14 = c[2'b11];
    abys_dumper_tmp15 = (abys_dumper_tmp11 & abys_dumper_tmp14);
    abys_dumper_tmp16 = (abys_dumper_tmp8 & abys_dumper_tmp15);
    abys_dumper_tmp18 = b[2'b10];
    abys_dumper_tmp20 = a[2'b10];
    abys_dumper_tmp21 = (abys_dumper_tmp18 & abys_dumper_tmp20);
    abys_dumper_tmp23 = d[2'b10];
    abys_dumper_tmp25 = c[2'b10];
    abys_dumper_tmp26 = (abys_dumper_tmp23 & abys_dumper_tmp25);
    abys_dumper_tmp27 = (abys_dumper_tmp21 & abys_dumper_tmp26);
    abys_dumper_tmp28 = b[1'b1];
    abys_dumper_tmp29 = a[1'b1];
    abys_dumper_tmp30 = (abys_dumper_tmp28 & abys_dumper_tmp29);
    abys_dumper_tmp31 = d[1'b1];
    abys_dumper_tmp32 = c[1'b1];
    abys_dumper_tmp33 = (abys_dumper_tmp31 & abys_dumper_tmp32);
    abys_dumper_tmp34 = (abys_dumper_tmp30 & abys_dumper_tmp33);
    abys_dumper_tmp35 = b[1'b0];
    abys_dumper_tmp36 = a[1'b0];
    abys_dumper_tmp37 = (abys_dumper_tmp35 & abys_dumper_tmp36);
    abys_dumper_tmp38 = d[1'b0];
    abys_dumper_tmp39 = c[1'b0];
    abys_dumper_tmp40 = (abys_dumper_tmp38 & abys_dumper_tmp39);
    abys_dumper_tmp41 = (abys_dumper_tmp37 & abys_dumper_tmp40);
    abys_dumper_tmp42 = {abys_dumper_tmp16, abys_dumper_tmp27, abys_dumper_tmp34, abys_dumper_tmp41};
    y = abys_dumper_tmp42;
  end
endmodule
