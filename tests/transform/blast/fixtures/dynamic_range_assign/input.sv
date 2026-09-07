module top(
  input  logic [1:0] base,
  input  logic [1:0] a,
  output logic [3:0] y
);
  always_comb begin
    y = '0;
    y[base +: 2] = a;
  end
endmodule
