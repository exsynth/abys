module top(
  input logic clk,
  input logic [7:0] d,
  output logic [7:0] q
);
  always_ff @(negedge clk)
    q <= d;
endmodule
