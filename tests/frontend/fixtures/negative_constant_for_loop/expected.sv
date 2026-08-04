module top (
  input [3:0] a,
  output  logic [3:0] y);



  always @(*)   begin
    logic abys_dumper_tmp12;
    logic signed [31:0] abys_dumper_tmp6;
    logic signed [31:0] abys_dumper_tmp11;
    logic signed [31:0] abys_dumper_tmp14;
    logic [31:0] abys_dumper_tmp15;
    logic [3:0] abys_dumper_tmp16;
    logic abys_dumper_tmp24;
    logic signed [31:0] abys_dumper_tmp23;
    logic signed [31:0] abys_dumper_tmp26;
    logic [31:0] abys_dumper_tmp27;
    logic [3:0] abys_dumper_tmp28;
    logic abys_dumper_tmp36;
    logic signed [31:0] abys_dumper_tmp35;
    logic signed [31:0] abys_dumper_tmp38;
    logic [31:0] abys_dumper_tmp39;
    logic [3:0] abys_dumper_tmp40;
    logic abys_dumper_tmp48;
    logic signed [31:0] abys_dumper_tmp47;
    logic signed [31:0] abys_dumper_tmp50;
    logic [31:0] abys_dumper_tmp51;
    logic [3:0] abys_dumper_tmp52;
    abys_dumper_tmp6 = (-32'sb10);
    abys_dumper_tmp11 = (abys_dumper_tmp6 + 32'sb10);
    abys_dumper_tmp12 = a[abys_dumper_tmp11];
    abys_dumper_tmp14 = (abys_dumper_tmp6 + 32'sb10);
    abys_dumper_tmp15 = (1'b0 + abys_dumper_tmp14);
    abys_dumper_tmp16 = 4'b0;
    abys_dumper_tmp16[abys_dumper_tmp15] = abys_dumper_tmp12;
    abys_dumper_tmp23 = (-32'sb1 + 32'sb10);
    abys_dumper_tmp24 = a[abys_dumper_tmp23];
    abys_dumper_tmp26 = (-32'sb1 + 32'sb10);
    abys_dumper_tmp27 = (1'b0 + abys_dumper_tmp26);
    abys_dumper_tmp28 = abys_dumper_tmp16;
    abys_dumper_tmp28[abys_dumper_tmp27] = abys_dumper_tmp24;
    abys_dumper_tmp35 = (32'sb0 + 32'sb10);
    abys_dumper_tmp36 = a[abys_dumper_tmp35];
    abys_dumper_tmp38 = (32'sb0 + 32'sb10);
    abys_dumper_tmp39 = (1'b0 + abys_dumper_tmp38);
    abys_dumper_tmp40 = abys_dumper_tmp28;
    abys_dumper_tmp40[abys_dumper_tmp39] = abys_dumper_tmp36;
    abys_dumper_tmp47 = (32'sb1 + 32'sb10);
    abys_dumper_tmp48 = a[abys_dumper_tmp47];
    abys_dumper_tmp50 = (32'sb1 + 32'sb10);
    abys_dumper_tmp51 = (1'b0 + abys_dumper_tmp50);
    abys_dumper_tmp52 = abys_dumper_tmp40;
    abys_dumper_tmp52[abys_dumper_tmp51] = abys_dumper_tmp48;
    y = abys_dumper_tmp52;
  end
endmodule
