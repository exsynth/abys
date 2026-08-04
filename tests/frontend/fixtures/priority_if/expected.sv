module top (
  input first,
  input second,
  input [7:0] a,
  input [7:0] b,
  input [7:0] c,
  output  logic [7:0] y);



  always @(*)   begin
    if (first) begin
      y = a;
    end else begin
      if (second) begin
        y = b;
      end else begin
        y = c;
      end
    end
  end
endmodule
