module top(
  input  logic [3:0] a,
  input  logic [3:0] b,
  output logic [3:0] q
);
  always_comb begin
    q[1:0] = a[1:0];
  end
  always_comb begin
    q[2:1] = b[2:1];
  end
endmodule
