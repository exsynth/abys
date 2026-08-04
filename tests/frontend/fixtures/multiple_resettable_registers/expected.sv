module top (
  input clk,
  input reset,
  input [7:0] d0,
  input [7:0] d1,
  output  logic [7:0] q0,
  output  logic [7:0] q1);



  always @(posedge clk or posedge reset) begin
    if (reset) begin
      begin
        q0 <= 8'b0;
      end
    end else begin
      begin
        q0 <= d0;
      end
    end
  end
  always @(posedge clk or posedge reset) begin
    if (reset) begin
      begin
        q1 <= 8'b0;
      end
    end else begin
      begin
        q1 <= d1;
      end
    end
  end
endmodule
