module top (
  input [7:0] lhs [0:2],
  input [7:0] rhs [0:2],
  input condition,
  output  logic [7:0] selected [0:2]);



  always @(*)   begin
    if (condition) begin
      selected = lhs;
    end else begin
      selected = rhs;
    end
  end
endmodule
