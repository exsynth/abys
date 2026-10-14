module top (
  input [3:0] a,
  input [3:0] b,
  input [3:0] c,
  output  logic [3:0] y);

  logic [3:0] abys_transformer_tmp0;
  logic [3:0] abys_transformer_tmp1;
  logic [3:0] abys_transformer_tmp2;
  logic [3:0] abys_transformer_tmp3;
  logic [3:0] abys_transformer_tmp4;
  logic [3:0] abys_transformer_tmp5;
  logic abys_transformer_tmp6;
  logic abys_transformer_tmp7;
  logic abys_transformer_tmp8;
  logic abys_transformer_tmp9;
  logic abys_transformer_tmp10;
  logic abys_transformer_tmp11;
  logic abys_transformer_tmp12;
  logic abys_transformer_tmp13;
  logic abys_transformer_tmp15;
  logic abys_transformer_tmp16;
  logic abys_transformer_tmp17;
  logic abys_transformer_tmp18;
  logic abys_transformer_tmp19;
  logic abys_transformer_tmp20;
  logic abys_transformer_tmp21;
  logic abys_transformer_tmp22;
  logic abys_transformer_tmp23;
  logic abys_transformer_tmp24;
  logic abys_transformer_tmp25;
  logic abys_transformer_tmp26;
  logic abys_transformer_tmp28;
  logic abys_transformer_tmp29;
  logic abys_transformer_tmp30;
  logic abys_transformer_tmp31;


  always @(*)   begin
    logic abys_dumper_tmp7;
    logic abys_dumper_tmp8;
    logic abys_dumper_tmp10;
    logic abys_dumper_tmp12;
    logic abys_dumper_tmp13;
    logic abys_dumper_tmp14;
    logic abys_dumper_tmp16;
    logic abys_dumper_tmp18;
    logic abys_dumper_tmp19;
    logic abys_dumper_tmp20;
    logic abys_dumper_tmp22;
    logic abys_dumper_tmp24;
    logic abys_dumper_tmp25;
    logic abys_dumper_tmp26;
    logic abys_dumper_tmp28;
    logic abys_dumper_tmp30;
    abys_dumper_tmp7 = a[1'b0];
    abys_dumper_tmp8 = a[1'b1];
    abys_dumper_tmp10 = a[2'b10];
    abys_dumper_tmp12 = a[2'b11];
    abys_dumper_tmp13 = b[1'b0];
    abys_dumper_tmp14 = b[1'b1];
    abys_dumper_tmp16 = b[2'b10];
    abys_dumper_tmp18 = b[2'b11];
    abys_dumper_tmp19 = abys_transformer_tmp2[1'b0];
    abys_dumper_tmp20 = abys_transformer_tmp2[1'b1];
    abys_dumper_tmp22 = abys_transformer_tmp2[2'b10];
    abys_dumper_tmp24 = abys_transformer_tmp2[2'b11];
    abys_dumper_tmp25 = c[1'b0];
    abys_dumper_tmp26 = c[1'b1];
    abys_dumper_tmp28 = c[2'b10];
    abys_dumper_tmp30 = c[2'b11];
    y = abys_transformer_tmp5;
    abys_transformer_tmp0 = a;
    abys_transformer_tmp1 = b;
    abys_transformer_tmp3 = abys_transformer_tmp2;
    abys_transformer_tmp4 = c;
    abys_transformer_tmp6 = abys_dumper_tmp7;
    abys_transformer_tmp7 = abys_dumper_tmp8;
    abys_transformer_tmp8 = abys_dumper_tmp10;
    abys_transformer_tmp9 = abys_dumper_tmp12;
    abys_transformer_tmp10 = abys_dumper_tmp13;
    abys_transformer_tmp11 = abys_dumper_tmp14;
    abys_transformer_tmp12 = abys_dumper_tmp16;
    abys_transformer_tmp13 = abys_dumper_tmp18;
    abys_transformer_tmp19 = abys_dumper_tmp19;
    abys_transformer_tmp20 = abys_dumper_tmp20;
    abys_transformer_tmp21 = abys_dumper_tmp22;
    abys_transformer_tmp22 = abys_dumper_tmp24;
    abys_transformer_tmp23 = abys_dumper_tmp25;
    abys_transformer_tmp24 = abys_dumper_tmp26;
    abys_transformer_tmp25 = abys_dumper_tmp28;
    abys_transformer_tmp26 = abys_dumper_tmp30;
  end
  logic abys_transformer_tmp14_10_0;
  XOR2 abys_transformer_tmp14_10 (
    .A(abys_transformer_tmp6),
    .B(abys_transformer_tmp10),
    .Y(abys_transformer_tmp14_10_0)
  );
  logic abys_transformer_tmp14_11_0;
  AND2 abys_transformer_tmp14_11 (
    .A(abys_transformer_tmp6),
    .B(abys_transformer_tmp10),
    .Y(abys_transformer_tmp14_11_0)
  );
  logic abys_transformer_tmp14_12_0;
  XOR2 abys_transformer_tmp14_12 (
    .A(abys_transformer_tmp7),
    .B(abys_transformer_tmp11),
    .Y(abys_transformer_tmp14_12_0)
  );
  logic abys_transformer_tmp14_13_0;
  XOR2 abys_transformer_tmp14_13 (
    .A(abys_transformer_tmp14_12_0),
    .B(abys_transformer_tmp14_11_0),
    .Y(abys_transformer_tmp14_13_0)
  );
  logic abys_transformer_tmp14_14_0;
  AND2 abys_transformer_tmp14_14 (
    .A(abys_transformer_tmp7),
    .B(abys_transformer_tmp11),
    .Y(abys_transformer_tmp14_14_0)
  );
  logic abys_transformer_tmp14_15_0;
  AND2 abys_transformer_tmp14_15 (
    .A(abys_transformer_tmp14_12_0),
    .B(abys_transformer_tmp14_11_0),
    .Y(abys_transformer_tmp14_15_0)
  );
  logic abys_transformer_tmp14_16_0;
  OR2 abys_transformer_tmp14_16 (
    .A(abys_transformer_tmp14_14_0),
    .B(abys_transformer_tmp14_15_0),
    .Y(abys_transformer_tmp14_16_0)
  );
  logic abys_transformer_tmp14_17_0;
  XOR2 abys_transformer_tmp14_17 (
    .A(abys_transformer_tmp8),
    .B(abys_transformer_tmp12),
    .Y(abys_transformer_tmp14_17_0)
  );
  logic abys_transformer_tmp14_18_0;
  XOR2 abys_transformer_tmp14_18 (
    .A(abys_transformer_tmp14_16_0),
    .B(abys_transformer_tmp14_17_0),
    .Y(abys_transformer_tmp14_18_0)
  );
  logic abys_transformer_tmp14_19_0;
  AND2 abys_transformer_tmp14_19 (
    .A(abys_transformer_tmp8),
    .B(abys_transformer_tmp12),
    .Y(abys_transformer_tmp14_19_0)
  );
  logic abys_transformer_tmp14_20_0;
  AND2 abys_transformer_tmp14_20 (
    .A(abys_transformer_tmp14_16_0),
    .B(abys_transformer_tmp14_17_0),
    .Y(abys_transformer_tmp14_20_0)
  );
  logic abys_transformer_tmp14_21_0;
  OR2 abys_transformer_tmp14_21 (
    .A(abys_transformer_tmp14_19_0),
    .B(abys_transformer_tmp14_20_0),
    .Y(abys_transformer_tmp14_21_0)
  );
  logic abys_transformer_tmp14_22_0;
  XOR2 abys_transformer_tmp14_22 (
    .A(abys_transformer_tmp9),
    .B(abys_transformer_tmp13),
    .Y(abys_transformer_tmp14_22_0)
  );
  logic abys_transformer_tmp14_23_0;
  XOR2 abys_transformer_tmp14_23 (
    .A(abys_transformer_tmp14_21_0),
    .B(abys_transformer_tmp14_22_0),
    .Y(abys_transformer_tmp14_23_0)
  );
  assign abys_transformer_tmp15 = abys_transformer_tmp14_10_0;
  assign abys_transformer_tmp16 = abys_transformer_tmp14_13_0;
  assign abys_transformer_tmp17 = abys_transformer_tmp14_18_0;
  assign abys_transformer_tmp18 = abys_transformer_tmp14_23_0;
  logic abys_transformer_tmp27_10_0;
  AND2 abys_transformer_tmp27_10 (
    .A(abys_transformer_tmp19),
    .B(abys_transformer_tmp23),
    .Y(abys_transformer_tmp27_10_0)
  );
  logic abys_transformer_tmp27_11_0;
  AND2 abys_transformer_tmp27_11 (
    .A(abys_transformer_tmp20),
    .B(abys_transformer_tmp23),
    .Y(abys_transformer_tmp27_11_0)
  );
  logic abys_transformer_tmp27_12_0;
  AND2 abys_transformer_tmp27_12 (
    .A(abys_transformer_tmp19),
    .B(abys_transformer_tmp24),
    .Y(abys_transformer_tmp27_12_0)
  );
  logic abys_transformer_tmp27_13_0;
  XOR2 abys_transformer_tmp27_13 (
    .A(abys_transformer_tmp27_11_0),
    .B(abys_transformer_tmp27_12_0),
    .Y(abys_transformer_tmp27_13_0)
  );
  logic abys_transformer_tmp27_14_0;
  AND2 abys_transformer_tmp27_14 (
    .A(abys_transformer_tmp27_11_0),
    .B(abys_transformer_tmp27_12_0),
    .Y(abys_transformer_tmp27_14_0)
  );
  logic abys_transformer_tmp27_15_0;
  AND2 abys_transformer_tmp27_15 (
    .A(abys_transformer_tmp21),
    .B(abys_transformer_tmp23),
    .Y(abys_transformer_tmp27_15_0)
  );
  logic abys_transformer_tmp27_16_0;
  AND2 abys_transformer_tmp27_16 (
    .A(abys_transformer_tmp20),
    .B(abys_transformer_tmp24),
    .Y(abys_transformer_tmp27_16_0)
  );
  logic abys_transformer_tmp27_17_0;
  XOR2 abys_transformer_tmp27_17 (
    .A(abys_transformer_tmp27_15_0),
    .B(abys_transformer_tmp27_16_0),
    .Y(abys_transformer_tmp27_17_0)
  );
  logic abys_transformer_tmp27_18_0;
  XOR2 abys_transformer_tmp27_18 (
    .A(abys_transformer_tmp27_17_0),
    .B(abys_transformer_tmp27_14_0),
    .Y(abys_transformer_tmp27_18_0)
  );
  logic abys_transformer_tmp27_19_0;
  AND2 abys_transformer_tmp27_19 (
    .A(abys_transformer_tmp19),
    .B(abys_transformer_tmp25),
    .Y(abys_transformer_tmp27_19_0)
  );
  logic abys_transformer_tmp27_20_0;
  XOR2 abys_transformer_tmp27_20 (
    .A(abys_transformer_tmp27_18_0),
    .B(abys_transformer_tmp27_19_0),
    .Y(abys_transformer_tmp27_20_0)
  );
  logic abys_transformer_tmp27_21_0;
  AND2 abys_transformer_tmp27_21 (
    .A(abys_transformer_tmp27_18_0),
    .B(abys_transformer_tmp27_19_0),
    .Y(abys_transformer_tmp27_21_0)
  );
  logic abys_transformer_tmp27_22_0;
  AND2 abys_transformer_tmp27_22 (
    .A(abys_transformer_tmp27_15_0),
    .B(abys_transformer_tmp27_16_0),
    .Y(abys_transformer_tmp27_22_0)
  );
  logic abys_transformer_tmp27_23_0;
  AND2 abys_transformer_tmp27_23 (
    .A(abys_transformer_tmp27_17_0),
    .B(abys_transformer_tmp27_14_0),
    .Y(abys_transformer_tmp27_23_0)
  );
  logic abys_transformer_tmp27_24_0;
  OR2 abys_transformer_tmp27_24 (
    .A(abys_transformer_tmp27_22_0),
    .B(abys_transformer_tmp27_23_0),
    .Y(abys_transformer_tmp27_24_0)
  );
  logic abys_transformer_tmp27_25_0;
  AND2 abys_transformer_tmp27_25 (
    .A(abys_transformer_tmp22),
    .B(abys_transformer_tmp23),
    .Y(abys_transformer_tmp27_25_0)
  );
  logic abys_transformer_tmp27_26_0;
  AND2 abys_transformer_tmp27_26 (
    .A(abys_transformer_tmp21),
    .B(abys_transformer_tmp24),
    .Y(abys_transformer_tmp27_26_0)
  );
  logic abys_transformer_tmp27_27_0;
  XOR2 abys_transformer_tmp27_27 (
    .A(abys_transformer_tmp27_25_0),
    .B(abys_transformer_tmp27_26_0),
    .Y(abys_transformer_tmp27_27_0)
  );
  logic abys_transformer_tmp27_28_0;
  XOR2 abys_transformer_tmp27_28 (
    .A(abys_transformer_tmp27_24_0),
    .B(abys_transformer_tmp27_27_0),
    .Y(abys_transformer_tmp27_28_0)
  );
  logic abys_transformer_tmp27_29_0;
  AND2 abys_transformer_tmp27_29 (
    .A(abys_transformer_tmp20),
    .B(abys_transformer_tmp25),
    .Y(abys_transformer_tmp27_29_0)
  );
  logic abys_transformer_tmp27_30_0;
  XOR2 abys_transformer_tmp27_30 (
    .A(abys_transformer_tmp27_28_0),
    .B(abys_transformer_tmp27_29_0),
    .Y(abys_transformer_tmp27_30_0)
  );
  logic abys_transformer_tmp27_31_0;
  XOR2 abys_transformer_tmp27_31 (
    .A(abys_transformer_tmp27_30_0),
    .B(abys_transformer_tmp27_21_0),
    .Y(abys_transformer_tmp27_31_0)
  );
  logic abys_transformer_tmp27_32_0;
  AND2 abys_transformer_tmp27_32 (
    .A(abys_transformer_tmp19),
    .B(abys_transformer_tmp26),
    .Y(abys_transformer_tmp27_32_0)
  );
  logic abys_transformer_tmp27_33_0;
  XOR2 abys_transformer_tmp27_33 (
    .A(abys_transformer_tmp27_31_0),
    .B(abys_transformer_tmp27_32_0),
    .Y(abys_transformer_tmp27_33_0)
  );
  assign abys_transformer_tmp28 = abys_transformer_tmp27_10_0;
  assign abys_transformer_tmp29 = abys_transformer_tmp27_13_0;
  assign abys_transformer_tmp30 = abys_transformer_tmp27_20_0;
  assign abys_transformer_tmp31 = abys_transformer_tmp27_33_0;
  always @(*) begin
    abys_transformer_tmp2 = {abys_transformer_tmp18, abys_transformer_tmp17, abys_transformer_tmp16, abys_transformer_tmp15};
  end
  always @(*) begin
    abys_transformer_tmp5 = {abys_transformer_tmp31, abys_transformer_tmp30, abys_transformer_tmp29, abys_transformer_tmp28};
  end
endmodule
