module child(
  input  logic [1:0] a [0:1][0:1],
  output logic [1:0] y [0:1][0:1]
);
  always_comb begin
    y = a;
  end
endmodule

module top(
  input  logic [1:0] a [0:1][0:1],
  output logic [1:0] y [0:1][0:1]
);
  child u_child(.a(a), .y(y));
endmodule
