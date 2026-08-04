module top(
  input  logic [1:0][1:0] a,
  output logic [1:0][1:0] y
);
  always_comb begin
    for (int i = 0; i < 2; i = i + 1)
      for (int j = 0; j < 2; j = j + 1)
        y[i][j] = a[i][j];
  end
endmodule
