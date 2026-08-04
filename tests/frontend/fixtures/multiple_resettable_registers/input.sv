module top(
  input logic clk,
  input logic reset,
  input logic [7:0] d0,
  input logic [7:0] d1,
  output logic [7:0] q0,
  output logic [7:0] q1
);
  always_ff @(posedge clk or posedge reset) begin
    if (reset) begin
      q0 <= '0;
      q1 <= '0;
    end else begin
      q0 <= d0;
      q1 <= d1;
    end
  end
endmodule
