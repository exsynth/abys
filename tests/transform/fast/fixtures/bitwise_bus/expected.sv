module top (
  input [3:0] a,
  input [3:0] b,
  input [3:0] c,
  input [3:0] d,
  output  logic [3:0] y);

  logic abys_transformer_tmp0;
  logic abys_transformer_tmp1;
  logic abys_transformer_tmp2;
  logic abys_transformer_tmp3;
  logic abys_transformer_tmp4;
  logic abys_transformer_tmp5;
  logic abys_transformer_tmp6;
  logic abys_transformer_tmp7;
  logic abys_transformer_tmp8;
  logic abys_transformer_tmp9;
  logic abys_transformer_tmp10;
  logic abys_transformer_tmp11;
  logic abys_transformer_tmp12;
  logic abys_transformer_tmp13;
  logic abys_transformer_tmp14;
  logic abys_transformer_tmp15;
  logic abys_transformer_tmp16;
  logic abys_transformer_tmp17;
  logic abys_transformer_tmp18;
  logic abys_transformer_tmp19;


  always @(*)   begin
    logic abys_dumper_tmp3;
    logic abys_dumper_tmp4;
    logic abys_dumper_tmp6;
    logic abys_dumper_tmp8;
    logic abys_dumper_tmp10;
    logic abys_dumper_tmp11;
    logic abys_dumper_tmp13;
    logic abys_dumper_tmp15;
    logic abys_dumper_tmp17;
    logic abys_dumper_tmp18;
    logic abys_dumper_tmp20;
    logic abys_dumper_tmp22;
    logic abys_dumper_tmp24;
    logic abys_dumper_tmp25;
    logic abys_dumper_tmp27;
    logic abys_dumper_tmp29;
    abys_dumper_tmp3 = a[1'b0];
    abys_dumper_tmp4 = a[1'b1];
    abys_dumper_tmp6 = a[2'b10];
    abys_dumper_tmp8 = a[2'b11];
    abys_dumper_tmp10 = b[1'b0];
    abys_dumper_tmp11 = b[1'b1];
    abys_dumper_tmp13 = b[2'b10];
    abys_dumper_tmp15 = b[2'b11];
    abys_dumper_tmp17 = c[1'b0];
    abys_dumper_tmp18 = c[1'b1];
    abys_dumper_tmp20 = c[2'b10];
    abys_dumper_tmp22 = c[2'b11];
    abys_dumper_tmp24 = d[1'b0];
    abys_dumper_tmp25 = d[1'b1];
    abys_dumper_tmp27 = d[2'b10];
    abys_dumper_tmp29 = d[2'b11];
    abys_transformer_tmp0 = abys_dumper_tmp3;
    abys_transformer_tmp1 = abys_dumper_tmp4;
    abys_transformer_tmp2 = abys_dumper_tmp6;
    abys_transformer_tmp3 = abys_dumper_tmp8;
    abys_transformer_tmp4 = abys_dumper_tmp10;
    abys_transformer_tmp5 = abys_dumper_tmp11;
    abys_transformer_tmp6 = abys_dumper_tmp13;
    abys_transformer_tmp7 = abys_dumper_tmp15;
    abys_transformer_tmp8 = abys_dumper_tmp17;
    abys_transformer_tmp9 = abys_dumper_tmp18;
    abys_transformer_tmp10 = abys_dumper_tmp20;
    abys_transformer_tmp11 = abys_dumper_tmp22;
    abys_transformer_tmp12 = abys_dumper_tmp24;
    abys_transformer_tmp13 = abys_dumper_tmp25;
    abys_transformer_tmp14 = abys_dumper_tmp27;
    abys_transformer_tmp15 = abys_dumper_tmp29;
  end
  assign abys_transformer_tmp16 = ((abys_transformer_tmp4 & abys_transformer_tmp0) & (abys_transformer_tmp12 & abys_transformer_tmp8));
  assign abys_transformer_tmp17 = ((abys_transformer_tmp5 & abys_transformer_tmp1) & (abys_transformer_tmp13 & abys_transformer_tmp9));
  assign abys_transformer_tmp18 = ((abys_transformer_tmp6 & abys_transformer_tmp2) & (abys_transformer_tmp14 & abys_transformer_tmp10));
  assign abys_transformer_tmp19 = ((abys_transformer_tmp7 & abys_transformer_tmp3) & (abys_transformer_tmp15 & abys_transformer_tmp11));
  always @(*) begin
    y = {abys_transformer_tmp19, abys_transformer_tmp18, abys_transformer_tmp17, abys_transformer_tmp16};
  end
endmodule
