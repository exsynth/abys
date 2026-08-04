module top (
  input clk,
  input [7:0] d,
  output  logic [7:0] q);



  always @(negedge clk) begin
    begin
      q <= d;
    end
  end
endmodule
