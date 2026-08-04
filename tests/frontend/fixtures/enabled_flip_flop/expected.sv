module top (
  input clk,
  input enable,
  input [7:0] d,
  output  logic [7:0] q);



  always @(posedge clk) begin
    begin
      if (enable) begin
        q <= d;
      end else begin
      end
    end
  end
endmodule
