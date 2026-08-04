module top(
  input logic [7:0] a,
  input logic [7:0] b,
  output logic [7:0] sum,
  output logic [7:0] difference
);
  always_comb begin
    sum = a + b;
    difference = a - b;
  end
endmodule
