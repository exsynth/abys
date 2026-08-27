module top (
  input [1:0] base,
  input [1:0] a,
  output  logic [3:0] y);

  logic [3:0] abys_transformer_tmp0;
  logic \abys_transformer_tmp1[0] ;
  logic \abys_transformer_tmp1[1] ;
  logic \abys_transformer_tmp2[0] ;
  logic \abys_transformer_tmp2[1] ;


  always @(*)   begin
    logic abys_dumper_tmp12;
    logic [1:0] abys_dumper_tmp5;
    logic [1:0] abys_dumper_tmp9;
    logic [3:0] abys_dumper_tmp11;
    logic abys_dumper_tmp13;
    logic abys_dumper_tmp15;
    logic abys_dumper_tmp17;
    abys_dumper_tmp5 = {\abys_transformer_tmp2[1] , \abys_transformer_tmp2[0] };
    abys_dumper_tmp9 = {\abys_transformer_tmp1[1] , \abys_transformer_tmp1[0] };
    abys_dumper_tmp11 = 4'b0;
    abys_dumper_tmp11[($signed(7'({1'b0, 1'b0})) + ($signed(7'({1'b0, abys_dumper_tmp9})) * $signed(7'($signed(2'sb1))))) +: 2'b10] = abys_dumper_tmp5;
    abys_dumper_tmp12 = ((abys_dumper_tmp11 >> (1'b0)) & {1{1'b1}});
    abys_dumper_tmp13 = ((abys_dumper_tmp11 >> (1'b1)) & {1{1'b1}});
    abys_dumper_tmp15 = ((abys_dumper_tmp11 >> (2'b10)) & {1{1'b1}});
    abys_dumper_tmp17 = ((abys_dumper_tmp11 >> (2'b11)) & {1{1'b1}});
    abys_transformer_tmp0[0] = abys_dumper_tmp12;
    abys_transformer_tmp0[1] = abys_dumper_tmp13;
    abys_transformer_tmp0[2] = abys_dumper_tmp15;
    abys_transformer_tmp0[3] = abys_dumper_tmp17;
  end
  always @(*) begin
    y = {abys_transformer_tmp0[3], abys_transformer_tmp0[2], abys_transformer_tmp0[1], abys_transformer_tmp0[0]};
  end
  always @(*) begin
    \abys_transformer_tmp1[0]  = base[0];
    \abys_transformer_tmp1[1]  = base[1];
  end
  always @(*) begin
    \abys_transformer_tmp2[0]  = a[0];
    \abys_transformer_tmp2[1]  = a[1];
  end
endmodule
