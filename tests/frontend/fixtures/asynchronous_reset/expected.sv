module top (
  input clk,
  input rst_n,
  input [7:0] d,
  output  logic [7:0] q);



  always @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
      begin
        q <= 8'b0;
      end
    end else begin
      begin
        q <= d;
      end
    end
  end
endmodule
