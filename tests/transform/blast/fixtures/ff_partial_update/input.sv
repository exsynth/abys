module top(
  input  logic       clk,
  input  logic       enable,
  input  logic [1:0] b,
  output logic [1:0] q
);
  always_ff @(posedge clk) begin
    q[0] <= enable ? b[0] : q[0];
    q[1] <= b[1];
  end
endmodule
