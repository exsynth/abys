module top(
  input logic [3:0] a,
  input logic [3:0] b,
  output logic [3:0] y
);
  always_comb begin
    for (int i = 0; i < 4; i = i + 1)
      y[i] = a[i] & b[i];
  end
endmodule
