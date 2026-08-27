module top (
  input clk,
  input [3:0] d,
  output  logic [3:0] q);

  logic [3:0] abys_transformer_tmp0;
  logic \abys_transformer_tmp1[0] ;
  logic \abys_transformer_tmp1[1] ;
  logic \abys_transformer_tmp1[2] ;
  logic \abys_transformer_tmp1[3] ;


  always @(*) begin
    q = {abys_transformer_tmp0[3], abys_transformer_tmp0[2], abys_transformer_tmp0[1], abys_transformer_tmp0[0]};
  end
  always @(*) begin
    \abys_transformer_tmp1[0]  = d[0];
    \abys_transformer_tmp1[1]  = d[1];
    \abys_transformer_tmp1[2]  = d[2];
    \abys_transformer_tmp1[3]  = d[3];
  end
  always @(posedge clk) begin
    abys_transformer_tmp0[0] <= \abys_transformer_tmp1[0] ;
  end
  always @(posedge clk) begin
    abys_transformer_tmp0[1] <= \abys_transformer_tmp1[1] ;
  end
  always @(posedge clk) begin
    abys_transformer_tmp0[2] <= \abys_transformer_tmp1[2] ;
  end
  always @(posedge clk) begin
    abys_transformer_tmp0[3] <= \abys_transformer_tmp1[3] ;
  end
endmodule
