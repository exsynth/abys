module top (
  input [3:0] a,
  input [3:0] b,
  output  logic [3:0] q);

  logic [3:0] abys_transformer_tmp0;
  logic \abys_transformer_tmp1[0] ;
  logic \abys_transformer_tmp1[1] ;
  logic \abys_transformer_tmp1[2] ;
  logic \abys_transformer_tmp1[3] ;
  logic \abys_transformer_tmp2[0] ;
  logic \abys_transformer_tmp2[1] ;
  logic \abys_transformer_tmp2[2] ;
  logic \abys_transformer_tmp2[3] ;


  always @(*) begin
    abys_transformer_tmp0[0] = \abys_transformer_tmp1[0] ;
    abys_transformer_tmp0[0] = abys_transformer_tmp0[0];
  end
  always @(*) begin
    abys_transformer_tmp0[1] = \abys_transformer_tmp1[1] ;
    abys_transformer_tmp0[1] = \abys_transformer_tmp2[1] ;
  end
  always @(*) begin
    abys_transformer_tmp0[2] = abys_transformer_tmp0[2];
    abys_transformer_tmp0[2] = \abys_transformer_tmp2[2] ;
  end
  always @(*) begin
    abys_transformer_tmp0[3] = abys_transformer_tmp0[3];
    abys_transformer_tmp0[3] = abys_transformer_tmp0[3];
  end
  always @(*) begin
    q = {abys_transformer_tmp0[3], abys_transformer_tmp0[2], abys_transformer_tmp0[1], abys_transformer_tmp0[0]};
  end
  always @(*) begin
    \abys_transformer_tmp1[0]  = a[0];
    \abys_transformer_tmp1[1]  = a[1];
    \abys_transformer_tmp1[2]  = a[2];
    \abys_transformer_tmp1[3]  = a[3];
  end
  always @(*) begin
    \abys_transformer_tmp2[0]  = b[0];
    \abys_transformer_tmp2[1]  = b[1];
    \abys_transformer_tmp2[2]  = b[2];
    \abys_transformer_tmp2[3]  = b[3];
  end
endmodule
