module top(
  input logic [3:0] a,
  output logic [3:0] y
);
  always_comb begin
    y = '0;
    for (int i = -2; i <= 1; i += 1)
      y[i + 2] = a[i + 2];
  end
endmodule
