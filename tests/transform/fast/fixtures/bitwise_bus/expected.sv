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
    logic abys_dumper_tmp5;
    logic abys_dumper_tmp6;
    logic abys_dumper_tmp8;
    logic abys_dumper_tmp9;
    logic abys_dumper_tmp10;
    logic abys_dumper_tmp11;
    logic abys_dumper_tmp13;
    logic abys_dumper_tmp14;
    logic abys_dumper_tmp15;
    logic abys_dumper_tmp16;
    logic abys_dumper_tmp18;
    logic abys_dumper_tmp19;
    logic abys_dumper_tmp20;
    logic abys_dumper_tmp21;
    abys_dumper_tmp3 = a[1'b0];
    abys_dumper_tmp4 = a[1'b1];
    abys_dumper_tmp5 = a[2'b10];
    abys_dumper_tmp6 = a[2'b11];
    abys_dumper_tmp8 = b[1'b0];
    abys_dumper_tmp9 = b[1'b1];
    abys_dumper_tmp10 = b[2'b10];
    abys_dumper_tmp11 = b[2'b11];
    abys_dumper_tmp13 = c[1'b0];
    abys_dumper_tmp14 = c[1'b1];
    abys_dumper_tmp15 = c[2'b10];
    abys_dumper_tmp16 = c[2'b11];
    abys_dumper_tmp18 = d[1'b0];
    abys_dumper_tmp19 = d[1'b1];
    abys_dumper_tmp20 = d[2'b10];
    abys_dumper_tmp21 = d[2'b11];
    abys_transformer_tmp0 = abys_dumper_tmp3;
    abys_transformer_tmp1 = abys_dumper_tmp4;
    abys_transformer_tmp2 = abys_dumper_tmp5;
    abys_transformer_tmp3 = abys_dumper_tmp6;
    abys_transformer_tmp4 = abys_dumper_tmp8;
    abys_transformer_tmp5 = abys_dumper_tmp9;
    abys_transformer_tmp6 = abys_dumper_tmp10;
    abys_transformer_tmp7 = abys_dumper_tmp11;
    abys_transformer_tmp8 = abys_dumper_tmp13;
    abys_transformer_tmp9 = abys_dumper_tmp14;
    abys_transformer_tmp10 = abys_dumper_tmp15;
    abys_transformer_tmp11 = abys_dumper_tmp16;
    abys_transformer_tmp12 = abys_dumper_tmp18;
    abys_transformer_tmp13 = abys_dumper_tmp19;
    abys_transformer_tmp14 = abys_dumper_tmp20;
    abys_transformer_tmp15 = abys_dumper_tmp21;
  end
  assign abys_transformer_tmp16 = ((abys_transformer_tmp4 & abys_transformer_tmp0) & (abys_transformer_tmp12 & abys_transformer_tmp8));
  assign abys_transformer_tmp17 = ((abys_transformer_tmp5 & abys_transformer_tmp1) & (abys_transformer_tmp13 & abys_transformer_tmp9));
  assign abys_transformer_tmp18 = ((abys_transformer_tmp6 & abys_transformer_tmp2) & (abys_transformer_tmp14 & abys_transformer_tmp10));
  assign abys_transformer_tmp19 = ((abys_transformer_tmp7 & abys_transformer_tmp3) & (abys_transformer_tmp15 & abys_transformer_tmp11));
  always @(*) begin
    y = {abys_transformer_tmp19, abys_transformer_tmp18, abys_transformer_tmp17, abys_transformer_tmp16};
  end
endmodule
