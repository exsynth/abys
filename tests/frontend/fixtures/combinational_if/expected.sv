module top (
  input select,
  input [7:0] a,
  input [7:0] b,
  output  logic [7:0] y);



  always @(*)   begin
    if (select) begin
      y = a;
    end else begin
      y = b;
    end
  end
endmodule
