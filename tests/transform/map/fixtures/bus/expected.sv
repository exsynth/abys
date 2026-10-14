module top (
  input [3:0] a,
  input [3:0] b,
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


  always @(*)   begin
    logic abys_dumper_tmp3;
    logic abys_dumper_tmp4;
    logic abys_dumper_tmp6;
    logic abys_dumper_tmp8;
    logic abys_dumper_tmp10;
    logic abys_dumper_tmp11;
    logic abys_dumper_tmp13;
    logic abys_dumper_tmp15;
    abys_dumper_tmp3 = a[1'b0];
    abys_dumper_tmp4 = a[1'b1];
    abys_dumper_tmp6 = a[2'b10];
    abys_dumper_tmp8 = a[2'b11];
    abys_dumper_tmp10 = b[1'b0];
    abys_dumper_tmp11 = b[1'b1];
    abys_dumper_tmp13 = b[2'b10];
    abys_dumper_tmp15 = b[2'b11];
    abys_transformer_tmp0 = abys_dumper_tmp3;
    abys_transformer_tmp1 = abys_dumper_tmp4;
    abys_transformer_tmp2 = abys_dumper_tmp6;
    abys_transformer_tmp3 = abys_dumper_tmp8;
    abys_transformer_tmp4 = abys_dumper_tmp10;
    abys_transformer_tmp5 = abys_dumper_tmp11;
    abys_transformer_tmp6 = abys_dumper_tmp13;
    abys_transformer_tmp7 = abys_dumper_tmp15;
  end
  logic abys_transformer_tmp12_10_0;
  AND2 abys_transformer_tmp12_10 (
    .A(abys_transformer_tmp0),
    .B(abys_transformer_tmp4),
    .Y(abys_transformer_tmp12_10_0)
  );
  logic abys_transformer_tmp12_11_0;
  AND2 abys_transformer_tmp12_11 (
    .A(abys_transformer_tmp1),
    .B(abys_transformer_tmp5),
    .Y(abys_transformer_tmp12_11_0)
  );
  logic abys_transformer_tmp12_12_0;
  AND2 abys_transformer_tmp12_12 (
    .A(abys_transformer_tmp2),
    .B(abys_transformer_tmp6),
    .Y(abys_transformer_tmp12_12_0)
  );
  logic abys_transformer_tmp12_13_0;
  AND2 abys_transformer_tmp12_13 (
    .A(abys_transformer_tmp3),
    .B(abys_transformer_tmp7),
    .Y(abys_transformer_tmp12_13_0)
  );
  assign abys_transformer_tmp8 = abys_transformer_tmp12_10_0;
  assign abys_transformer_tmp9 = abys_transformer_tmp12_11_0;
  assign abys_transformer_tmp10 = abys_transformer_tmp12_12_0;
  assign abys_transformer_tmp11 = abys_transformer_tmp12_13_0;
  always @(*) begin
    y = {abys_transformer_tmp11, abys_transformer_tmp10, abys_transformer_tmp9, abys_transformer_tmp8};
  end
endmodule
