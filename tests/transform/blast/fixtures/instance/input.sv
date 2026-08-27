module child(
  input  logic [3:0] a,
  output logic [3:0] y
);
  always_comb begin
    y = a + 4'd1;
  end
endmodule

module top(
  input  logic [3:0] a,
  input  logic       select,
  output logic [3:0] y
);
  logic [3:0] child_y;

  child u_child (
    .a(a),
    .y(child_y)
  );

  always_comb begin
    y = select ? child_y : a;
  end
endmodule
