module top (
  input [3:0] a [0:1],
  output  logic [3:0] y [0:1]);


  child u_child (
    .a(a),
    .y(y)  );

endmodule

module child (
  input [3:0] a [0:1],
  output  logic [3:0] y [0:1]);

  logic [3:0] abys_transformer_tmp0 [0:1];
  logic \abys_transformer_tmp1[0] ;
  logic \abys_transformer_tmp1[1] ;
  logic \abys_transformer_tmp1[2] ;
  logic \abys_transformer_tmp1[3] ;
  logic \abys_transformer_tmp1[4] ;
  logic \abys_transformer_tmp1[5] ;
  logic \abys_transformer_tmp1[6] ;
  logic \abys_transformer_tmp1[7] ;


  always @(*)   begin
    logic abys_dumper_tmp24;
    logic [3:0] abys_dumper_tmp22 [0:1];
    logic [7:0] abys_dumper_tmp10;
    logic [3:0] abys_dumper_tmp11 [0:1];
    logic [7:0] abys_dumper_tmp20;
    logic [3:0] abys_dumper_tmp21 [0:1];
    logic [7:0] abys_dumper_tmp23;
    logic abys_dumper_tmp25;
    logic abys_dumper_tmp27;
    logic abys_dumper_tmp29;
    logic abys_dumper_tmp31;
    logic abys_dumper_tmp33;
    logic abys_dumper_tmp35;
    logic abys_dumper_tmp37;
    abys_dumper_tmp10 = {abys_transformer_tmp0[1][3], abys_transformer_tmp0[1][2], abys_transformer_tmp0[1][1], abys_transformer_tmp0[1][0], abys_transformer_tmp0[0][3], abys_transformer_tmp0[0][2], abys_transformer_tmp0[0][1], abys_transformer_tmp0[0][0]};
    abys_dumper_tmp11[0] = {abys_dumper_tmp10}[0 +: 4];
    abys_dumper_tmp11[1] = {abys_dumper_tmp10}[4 +: 4];
    abys_dumper_tmp22 = abys_dumper_tmp11;
    abys_dumper_tmp20 = {\abys_transformer_tmp1[7] , \abys_transformer_tmp1[6] , \abys_transformer_tmp1[5] , \abys_transformer_tmp1[4] , \abys_transformer_tmp1[3] , \abys_transformer_tmp1[2] , \abys_transformer_tmp1[1] , \abys_transformer_tmp1[0] };
    abys_dumper_tmp21[0] = {abys_dumper_tmp20}[0 +: 4];
    abys_dumper_tmp21[1] = {abys_dumper_tmp20}[4 +: 4];
    abys_dumper_tmp22 = abys_dumper_tmp21;
    abys_dumper_tmp23[0 +: 4] = abys_dumper_tmp22[0];
    abys_dumper_tmp23[4 +: 4] = abys_dumper_tmp22[1];
    abys_dumper_tmp24 = ((abys_dumper_tmp23 >> (1'b0)) & {1{1'b1}});
    abys_dumper_tmp25 = ((abys_dumper_tmp23 >> (1'b1)) & {1{1'b1}});
    abys_dumper_tmp27 = ((abys_dumper_tmp23 >> (2'b10)) & {1{1'b1}});
    abys_dumper_tmp29 = ((abys_dumper_tmp23 >> (2'b11)) & {1{1'b1}});
    abys_dumper_tmp31 = ((abys_dumper_tmp23 >> (3'b100)) & {1{1'b1}});
    abys_dumper_tmp33 = ((abys_dumper_tmp23 >> (3'b101)) & {1{1'b1}});
    abys_dumper_tmp35 = ((abys_dumper_tmp23 >> (3'b110)) & {1{1'b1}});
    abys_dumper_tmp37 = ((abys_dumper_tmp23 >> (3'b111)) & {1{1'b1}});
    abys_transformer_tmp0[0][0] = abys_dumper_tmp24;
    abys_transformer_tmp0[0][1] = abys_dumper_tmp25;
    abys_transformer_tmp0[0][2] = abys_dumper_tmp27;
    abys_transformer_tmp0[0][3] = abys_dumper_tmp29;
    abys_transformer_tmp0[1][0] = abys_dumper_tmp31;
    abys_transformer_tmp0[1][1] = abys_dumper_tmp33;
    abys_transformer_tmp0[1][2] = abys_dumper_tmp35;
    abys_transformer_tmp0[1][3] = abys_dumper_tmp37;
  end
  always @(*) begin
    y[0] = {abys_transformer_tmp0[0][3], abys_transformer_tmp0[0][2], abys_transformer_tmp0[0][1], abys_transformer_tmp0[0][0]};
    y[1] = {abys_transformer_tmp0[1][3], abys_transformer_tmp0[1][2], abys_transformer_tmp0[1][1], abys_transformer_tmp0[1][0]};
  end
  always @(*) begin
    \abys_transformer_tmp1[0]  = a[0][0];
    \abys_transformer_tmp1[1]  = a[0][1];
    \abys_transformer_tmp1[2]  = a[0][2];
    \abys_transformer_tmp1[3]  = a[0][3];
    \abys_transformer_tmp1[4]  = a[1][0];
    \abys_transformer_tmp1[5]  = a[1][1];
    \abys_transformer_tmp1[6]  = a[1][2];
    \abys_transformer_tmp1[7]  = a[1][3];
  end
endmodule
