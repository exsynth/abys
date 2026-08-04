module child #(
  parameter int WIDTH = 8
)(
  input logic [WIDTH-1:0] a,
  output logic [WIDTH-1:0] y
);
  assign y = a + 1'b1;
endmodule

module top(
  input logic [3:0] a,
  output logic [3:0] y
);
  child #(.WIDTH(4)) u_child(.a(a), .y(y));
endmodule
