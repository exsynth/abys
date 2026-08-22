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
    logic abys_dumper_tmp28;
    logic abys_dumper_tmp30;
    logic abys_dumper_tmp31;
    logic [3:0] abys_dumper_tmp33;
    logic abys_dumper_tmp40;
    logic abys_dumper_tmp42;
    logic abys_dumper_tmp43;
    logic [3:0] abys_dumper_tmp45;
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
    abys_dumper_tmp28 = a[2'b10];
    abys_dumper_tmp30 = b[2'b10];
    abys_dumper_tmp31 = (abys_dumper_tmp28 & abys_dumper_tmp30);
    abys_dumper_tmp33 = abys_dumper_tmp21;
    abys_dumper_tmp33[2'b10] = abys_dumper_tmp31;
    abys_dumper_tmp40 = a[2'b11];
    abys_dumper_tmp42 = b[2'b11];
    abys_dumper_tmp43 = (abys_dumper_tmp40 & abys_dumper_tmp42);
    abys_dumper_tmp45 = abys_dumper_tmp33;
    abys_dumper_tmp45[2'b11] = abys_dumper_tmp43;
    y = abys_dumper_tmp45;
  end
endmodule
