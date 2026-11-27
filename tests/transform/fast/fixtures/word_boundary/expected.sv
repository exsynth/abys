module top (
  input [3:0] a,
  input [3:0] b,
  input select,
  output  logic y);

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
    logic [3:0] abys_dumper_tmp12;
    logic abys_dumper_tmp14;
    logic abys_dumper_tmp15;
    logic abys_dumper_tmp16;
    logic abys_dumper_tmp18;
    logic abys_dumper_tmp19;
    logic abys_dumper_tmp20;
    logic abys_dumper_tmp21;
    logic abys_dumper_tmp22;
    logic abys_dumper_tmp23;
    abys_dumper_tmp3 = a[1'b0];
    abys_dumper_tmp4 = a[1'b1];
    abys_dumper_tmp5 = a[2'b10];
    abys_dumper_tmp6 = a[2'b11];
    abys_dumper_tmp8 = b[1'b0];
    abys_dumper_tmp9 = b[1'b1];
    abys_dumper_tmp10 = b[2'b10];
    abys_dumper_tmp11 = b[2'b11];
    abys_dumper_tmp12 = (a + b);
    abys_dumper_tmp13 = ((abys_dumper_tmp12 >> (1'b0)) & {1{1'b1}});
    abys_dumper_tmp14 = ((abys_dumper_tmp12 >> (1'b1)) & {1{1'b1}});
    abys_dumper_tmp15 = ((abys_dumper_tmp12 >> (2'b10)) & {1{1'b1}});
    abys_dumper_tmp16 = ((abys_dumper_tmp12 >> (2'b11)) & {1{1'b1}});
    abys_dumper_tmp18 = ((4'b111 >> (1'b0)) & {1{1'b1}});
    abys_dumper_tmp19 = ((4'b111 >> (1'b1)) & {1{1'b1}});
    abys_dumper_tmp20 = ((4'b111 >> (2'b10)) & {1{1'b1}});
    abys_dumper_tmp21 = ((4'b111 >> (2'b11)) & {1{1'b1}});
    abys_dumper_tmp22 = (abys_dumper_tmp12 == 4'b111);
    abys_dumper_tmp23 = (a == b);
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
    abys_transformer_tmp16 = abys_dumper_tmp22;
    abys_transformer_tmp17 = abys_dumper_tmp23;
  end
  assign y = ~((~((select & abys_transformer_tmp16)) & ~((~(select) & abys_transformer_tmp17))));
endmodule
