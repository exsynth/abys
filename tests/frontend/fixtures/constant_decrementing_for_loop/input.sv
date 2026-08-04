module top(
  input logic [3:0] a,
  output logic [3:0] y
);
  always_comb begin
    y = '0;
    for (int i = 3; i >= 0; i -= 1)
      y[i] = a[i];
  end
endmodule
