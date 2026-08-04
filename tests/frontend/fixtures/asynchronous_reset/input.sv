module top(
  input logic clk,
  input logic rst_n,
  input logic [7:0] d,
  output logic [7:0] q
);
  always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n)
      q <= '0;
    else
      q <= d;
  end
endmodule
