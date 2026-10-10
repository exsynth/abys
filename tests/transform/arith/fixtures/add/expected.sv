module top (
  input [3:0] a,
  input [3:0] b,
  output  logic [3:0] y);

  logic [3:0] abys_transformer_tmp0;
  logic [3:0] abys_transformer_tmp1;
  logic [3:0] abys_transformer_tmp2;
  logic abys_transformer_tmp3;
  logic abys_transformer_tmp4;
  logic abys_transformer_tmp5;
  logic abys_transformer_tmp6;
  logic abys_transformer_tmp7;
  logic abys_transformer_tmp8;
  logic abys_transformer_tmp9;
  logic abys_transformer_tmp10;
  logic abys_transformer_tmp12;
  logic abys_transformer_tmp13;
  logic abys_transformer_tmp14;
  logic abys_transformer_tmp15;


  always @(*)   begin
    logic abys_dumper_tmp5;
    logic abys_dumper_tmp6;
    logic abys_dumper_tmp8;
    logic abys_dumper_tmp10;
    logic abys_dumper_tmp11;
    logic abys_dumper_tmp12;
    logic abys_dumper_tmp14;
    logic abys_dumper_tmp16;
    abys_dumper_tmp5 = a[1'b0];
    abys_dumper_tmp6 = a[1'b1];
    abys_dumper_tmp8 = a[2'b10];
    abys_dumper_tmp10 = a[2'b11];
    abys_dumper_tmp11 = b[1'b0];
    abys_dumper_tmp12 = b[1'b1];
    abys_dumper_tmp14 = b[2'b10];
    abys_dumper_tmp16 = b[2'b11];
    y = abys_transformer_tmp2;
    abys_transformer_tmp0 = a;
    abys_transformer_tmp1 = b;
    abys_transformer_tmp3 = abys_dumper_tmp5;
    abys_transformer_tmp4 = abys_dumper_tmp6;
    abys_transformer_tmp5 = abys_dumper_tmp8;
    abys_transformer_tmp6 = abys_dumper_tmp10;
    abys_transformer_tmp7 = abys_dumper_tmp11;
    abys_transformer_tmp8 = abys_dumper_tmp12;
    abys_transformer_tmp9 = abys_dumper_tmp14;
    abys_transformer_tmp10 = abys_dumper_tmp16;
  end
  logic abys_transformer_tmp11_10_0;
  XOR2 abys_transformer_tmp11_10 (
    .A(abys_transformer_tmp3),
    .B(abys_transformer_tmp7),
    .Y(abys_transformer_tmp11_10_0)
  );
  logic abys_transformer_tmp11_11_0;
  AND2 abys_transformer_tmp11_11 (
    .A(abys_transformer_tmp3),
    .B(abys_transformer_tmp7),
    .Y(abys_transformer_tmp11_11_0)
  );
  logic abys_transformer_tmp11_12_0;
  XOR2 abys_transformer_tmp11_12 (
    .A(abys_transformer_tmp4),
    .B(abys_transformer_tmp8),
    .Y(abys_transformer_tmp11_12_0)
  );
  logic abys_transformer_tmp11_13_0;
  XOR2 abys_transformer_tmp11_13 (
    .A(abys_transformer_tmp11_12_0),
    .B(abys_transformer_tmp11_11_0),
    .Y(abys_transformer_tmp11_13_0)
  );
  logic abys_transformer_tmp11_14_0;
  AND2 abys_transformer_tmp11_14 (
    .A(abys_transformer_tmp4),
    .B(abys_transformer_tmp8),
    .Y(abys_transformer_tmp11_14_0)
  );
  logic abys_transformer_tmp11_15_0;
  AND2 abys_transformer_tmp11_15 (
    .A(abys_transformer_tmp11_12_0),
    .B(abys_transformer_tmp11_11_0),
    .Y(abys_transformer_tmp11_15_0)
  );
  logic abys_transformer_tmp11_16_0;
  OR2 abys_transformer_tmp11_16 (
    .A(abys_transformer_tmp11_14_0),
    .B(abys_transformer_tmp11_15_0),
    .Y(abys_transformer_tmp11_16_0)
  );
  logic abys_transformer_tmp11_17_0;
  XOR2 abys_transformer_tmp11_17 (
    .A(abys_transformer_tmp5),
    .B(abys_transformer_tmp9),
    .Y(abys_transformer_tmp11_17_0)
  );
  logic abys_transformer_tmp11_18_0;
  XOR2 abys_transformer_tmp11_18 (
    .A(abys_transformer_tmp11_16_0),
    .B(abys_transformer_tmp11_17_0),
    .Y(abys_transformer_tmp11_18_0)
  );
  logic abys_transformer_tmp11_19_0;
  AND2 abys_transformer_tmp11_19 (
    .A(abys_transformer_tmp5),
    .B(abys_transformer_tmp9),
    .Y(abys_transformer_tmp11_19_0)
  );
  logic abys_transformer_tmp11_20_0;
  AND2 abys_transformer_tmp11_20 (
    .A(abys_transformer_tmp11_16_0),
    .B(abys_transformer_tmp11_17_0),
    .Y(abys_transformer_tmp11_20_0)
  );
  logic abys_transformer_tmp11_21_0;
  OR2 abys_transformer_tmp11_21 (
    .A(abys_transformer_tmp11_19_0),
    .B(abys_transformer_tmp11_20_0),
    .Y(abys_transformer_tmp11_21_0)
  );
  logic abys_transformer_tmp11_22_0;
  XOR2 abys_transformer_tmp11_22 (
    .A(abys_transformer_tmp6),
    .B(abys_transformer_tmp10),
    .Y(abys_transformer_tmp11_22_0)
  );
  logic abys_transformer_tmp11_23_0;
  XOR2 abys_transformer_tmp11_23 (
    .A(abys_transformer_tmp11_21_0),
    .B(abys_transformer_tmp11_22_0),
    .Y(abys_transformer_tmp11_23_0)
  );
  assign abys_transformer_tmp12 = abys_transformer_tmp11_10_0;
  assign abys_transformer_tmp13 = abys_transformer_tmp11_13_0;
  assign abys_transformer_tmp14 = abys_transformer_tmp11_18_0;
  assign abys_transformer_tmp15 = abys_transformer_tmp11_23_0;
  always @(*) begin
    abys_transformer_tmp2 = {abys_transformer_tmp15, abys_transformer_tmp14, abys_transformer_tmp13, abys_transformer_tmp12};
  end
endmodule
