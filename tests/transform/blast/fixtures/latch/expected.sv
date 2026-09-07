module top (
  input [1:0] b,
  input enable,
  output  logic [1:0] q);

  logic [1:0] abys_transformer_tmp0;
  logic \abys_transformer_tmp1[0] ;
  logic \abys_transformer_tmp1[1] ;


  always @(*)   begin
    if (enable) begin
      abys_transformer_tmp0[0] = \abys_transformer_tmp1[0] ;
    end else begin
    end
  end
  always @(*) begin
    q = {\abys_transformer_tmp1[1] , abys_transformer_tmp0[0]};
  end
  always @(*) begin
    \abys_transformer_tmp1[0]  = b[0];
    \abys_transformer_tmp1[1]  = b[1];
  end
endmodule
