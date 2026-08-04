module top (
  input [3:0] a,
  output  logic [3:0] y);



  always @(*)   begin
    logic abys_dumper_tmp8;
    logic [31:0] abys_dumper_tmp9;
    logic [3:0] abys_dumper_tmp10;
    logic abys_dumper_tmp16;
    logic [31:0] abys_dumper_tmp17;
    logic [3:0] abys_dumper_tmp18;
    logic abys_dumper_tmp24;
    logic [31:0] abys_dumper_tmp25;
    logic [3:0] abys_dumper_tmp26;
    logic abys_dumper_tmp32;
    logic [31:0] abys_dumper_tmp33;
    logic [3:0] abys_dumper_tmp34;
    abys_dumper_tmp8 = a[32'sb11];
    abys_dumper_tmp9 = (1'b0 + 32'sb11);
    abys_dumper_tmp10 = 4'b0;
    abys_dumper_tmp10[abys_dumper_tmp9] = abys_dumper_tmp8;
    abys_dumper_tmp16 = a[32'sb10];
    abys_dumper_tmp17 = (1'b0 + 32'sb10);
    abys_dumper_tmp18 = abys_dumper_tmp10;
    abys_dumper_tmp18[abys_dumper_tmp17] = abys_dumper_tmp16;
    abys_dumper_tmp24 = a[32'sb1];
    abys_dumper_tmp25 = (1'b0 + 32'sb1);
    abys_dumper_tmp26 = abys_dumper_tmp18;
    abys_dumper_tmp26[abys_dumper_tmp25] = abys_dumper_tmp24;
    abys_dumper_tmp32 = a[32'sb0];
    abys_dumper_tmp33 = (1'b0 + 32'sb0);
    abys_dumper_tmp34 = abys_dumper_tmp26;
    abys_dumper_tmp34[abys_dumper_tmp33] = abys_dumper_tmp32;
    y = abys_dumper_tmp34;
  end
endmodule
