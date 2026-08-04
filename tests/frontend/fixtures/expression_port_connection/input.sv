module child(
  input logic [7:0] a,
  output logic [7:0] y
);
  assign y = ~a;
endmodule

module top(
  input logic [7:0] a,
  input logic [7:0] b,
  output logic [7:0] y
);
  child u_child(.a(a + b), .y(y));
endmodule
