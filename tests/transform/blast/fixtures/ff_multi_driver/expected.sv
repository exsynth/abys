module top (
  input clk,
  input [1:0] a,
  input [1:0] b,
  output  logic [1:0] q);

  logic [1:0] abys_transformer_tmp0;
  logic \abys_transformer_tmp1[0] ;
  logic \abys_transformer_tmp1[1] ;
  logic \abys_transformer_tmp2[0] ;
  logic \abys_transformer_tmp2[1] ;


  always @(*) begin
    q = {abys_transformer_tmp0[1], abys_transformer_tmp0[0]};
  end
  always @(*) begin
    \abys_transformer_tmp1[0]  = a[0];
    \abys_transformer_tmp1[1]  = a[1];
  end
  always @(*) begin
    \abys_transformer_tmp2[0]  = b[0];
    \abys_transformer_tmp2[1]  = b[1];
  end
  always @(posedge clk) begin
    abys_transformer_tmp0[0] <= \abys_transformer_tmp1[0] ;
  end
  always @(posedge clk) begin
    abys_transformer_tmp0[1] <= \abys_transformer_tmp2[1] ;
  end
endmodule
