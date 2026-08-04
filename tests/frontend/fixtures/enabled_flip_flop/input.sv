module top(
  input logic clk,
  input logic enable,
  input logic [7:0] d,
  output logic [7:0] q
);
  always_ff @(posedge clk) begin
    if (enable)
      q <= d;
  end
endmodule
