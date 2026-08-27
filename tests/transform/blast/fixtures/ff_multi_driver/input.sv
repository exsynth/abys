module top(
  input  logic       clk,
  input  logic [1:0] a,
  input  logic [1:0] b,
  output logic [1:0] q
);
  always_ff @(posedge clk) begin
    q[0] <= a[0];
  end
  always_ff @(posedge clk) begin
    q[1] <= b[1];
  end
endmodule
