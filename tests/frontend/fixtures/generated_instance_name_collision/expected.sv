module top (
  input a_i,
  input [1:0] b_i,
  output  logic a_o,
  output  logic [1:0] b_o);


  child i_child_abys_gen_a_genblk0_0 (
    .d_i(a_i),
    .d_o(a_o)  );
  child_abys_variant1 i_child_abys_gen_b_genblk0_0 (
    .d_i(b_i),
    .d_o(b_o)  );

endmodule

module child (
  input d_i,
  output  logic d_o);



  always @(*)   begin
    d_o = d_i;
  end
endmodule

module child_abys_variant1 (
  input [1:0] d_i,
  output  logic [1:0] d_o);



  always @(*)   begin
    d_o = d_i;
  end
endmodule
