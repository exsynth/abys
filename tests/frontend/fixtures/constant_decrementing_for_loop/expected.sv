module top (
  input [3:0] a,
  output  logic [3:0] y);



  always @(*)   begin
    logic abys_dumper_tmp9;
    logic [3:0] abys_dumper_tmp11;
    logic abys_dumper_tmp18;
    logic [3:0] abys_dumper_tmp20;
    logic abys_dumper_tmp26;
    logic [3:0] abys_dumper_tmp27;
    logic abys_dumper_tmp33;
    logic [3:0] abys_dumper_tmp34;
    abys_dumper_tmp9 = a[2'b11];
    abys_dumper_tmp11 = 4'b0;
    abys_dumper_tmp11[2'b11] = abys_dumper_tmp9;
    abys_dumper_tmp18 = a[2'b10];
    abys_dumper_tmp20 = abys_dumper_tmp11;
    abys_dumper_tmp20[2'b10] = abys_dumper_tmp18;
    abys_dumper_tmp26 = a[1'b1];
    abys_dumper_tmp27 = abys_dumper_tmp20;
    abys_dumper_tmp27[1'b1] = abys_dumper_tmp26;
    abys_dumper_tmp33 = a[1'b0];
    abys_dumper_tmp34 = abys_dumper_tmp27;
    abys_dumper_tmp34[1'b0] = abys_dumper_tmp33;
    y = abys_dumper_tmp34;
  end
endmodule
