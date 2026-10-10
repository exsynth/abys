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
  logic abys_transformer_tmp12;
  logic abys_transformer_tmp13;
  logic abys_transformer_tmp14;
  logic abys_transformer_tmp15;


  always @(*)   begin
    logic [3:0] abys_dumper_tmp6;
    logic [3:0] abys_dumper_tmp8;
    logic abys_dumper_tmp10;
    logic abys_dumper_tmp11;
    logic abys_dumper_tmp13;
    logic abys_dumper_tmp15;
    logic abys_dumper_tmp16;
    logic abys_dumper_tmp17;
    logic abys_dumper_tmp19;
    logic abys_dumper_tmp21;
    logic abys_dumper_tmp22;
    logic abys_dumper_tmp23;
    logic abys_dumper_tmp25;
    logic abys_dumper_tmp27;
    abys_dumper_tmp6 = {abys_transformer_tmp15, abys_transformer_tmp14, abys_transformer_tmp13, abys_transformer_tmp12};
    abys_dumper_tmp8 = (abys_dumper_tmp6 + b);
    abys_dumper_tmp10 = a[1'b0];
    abys_dumper_tmp11 = a[1'b1];
    abys_dumper_tmp13 = a[2'b10];
    abys_dumper_tmp15 = a[2'b11];
    abys_dumper_tmp16 = b[1'b0];
    abys_dumper_tmp17 = b[1'b1];
    abys_dumper_tmp19 = b[2'b10];
    abys_dumper_tmp21 = b[2'b11];
    abys_dumper_tmp22 = ((abys_dumper_tmp8 >> (1'b0)) & {1{1'b1}});
    abys_dumper_tmp23 = ((abys_dumper_tmp8 >> (1'b1)) & {1{1'b1}});
    abys_dumper_tmp25 = ((abys_dumper_tmp8 >> (2'b10)) & {1{1'b1}});
    abys_dumper_tmp27 = ((abys_dumper_tmp8 >> (2'b11)) & {1{1'b1}});
    y = abys_dumper_tmp8;
    abys_transformer_tmp0 = abys_dumper_tmp10;
    abys_transformer_tmp1 = abys_dumper_tmp11;
    abys_transformer_tmp2 = abys_dumper_tmp13;
    abys_transformer_tmp3 = abys_dumper_tmp15;
    abys_transformer_tmp4 = abys_dumper_tmp16;
    abys_transformer_tmp5 = abys_dumper_tmp17;
    abys_transformer_tmp6 = abys_dumper_tmp19;
    abys_transformer_tmp7 = abys_dumper_tmp21;
    abys_transformer_tmp8 = abys_dumper_tmp22;
    abys_transformer_tmp9 = abys_dumper_tmp23;
    abys_transformer_tmp10 = abys_dumper_tmp25;
    abys_transformer_tmp11 = abys_dumper_tmp27;
  end
  assign abys_transformer_tmp12 = (abys_transformer_tmp0 & abys_transformer_tmp0);
  assign abys_transformer_tmp13 = (abys_transformer_tmp1 & abys_transformer_tmp1);
  assign abys_transformer_tmp14 = (abys_transformer_tmp2 & abys_transformer_tmp2);
  assign abys_transformer_tmp15 = (abys_transformer_tmp3 & abys_transformer_tmp3);
endmodule
