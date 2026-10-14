module top (
  input [3:0] a,
  input [3:0] b,
  output  logic [3:0] y);



  logic [3:0] abys_transformer_tmp0_4_0;
  AND2 abys_transformer_tmp0_4 [3:0] (
    .A(a),
    .B(b),
    .Y(abys_transformer_tmp0_4_0)
  );
  assign y = abys_transformer_tmp0_4_0;
endmodule
