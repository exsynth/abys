module top (
  input [7:0] a,
  input [7:0] b,
  output  logic [7:0] y);

  logic [7:0] abys_builder_tmp0;

  child u_child (
    .a(abys_builder_tmp0),
    .y(y)  );

  always @(*)   begin
    logic [7:0] abys_dumper_tmp4;
    abys_dumper_tmp4 = (a + b);
    abys_builder_tmp0 = abys_dumper_tmp4;
  end
endmodule

module child (
  input [7:0] a,
  output  logic [7:0] y);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp3;
    abys_dumper_tmp3 = (~a);
    y = abys_dumper_tmp3;
  end
endmodule
