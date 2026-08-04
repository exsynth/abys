module top (
  input [3:0] a,
  output  logic [3:0] y);


  child u_child (
    .a(a),
    .y(y)  );

endmodule

module child (
  input [3:0] a,
  output  logic [3:0] y);



  always @(*)   begin
    logic [3:0] abys_dumper_tmp3;
    logic [3:0] abys_dumper_tmp4;
    abys_dumper_tmp3 = 1'b1;
    abys_dumper_tmp4 = (a + abys_dumper_tmp3);
    y = abys_dumper_tmp4;
  end
endmodule
