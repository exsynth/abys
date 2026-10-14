module top (
  input a,
  input b,
  output  logic y);



  logic abys_transformer_tmp0_4_0;
  AND2 abys_transformer_tmp0_4 (
    .A(a),
    .B(b),
    .Y(abys_transformer_tmp0_4_0)
  );
  assign y = abys_transformer_tmp0_4_0;
endmodule
