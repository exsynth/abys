module top(
  input  logic [7:0] a,
  input  logic [2:0] shift,
  output logic [7:0] y
);
  always_comb begin
    y = a;
    y += 8'd3;
    y -= 8'd1;
    y &= 8'hfe;
    y |= 8'h10;
    y ^= 8'h55;
    y <<= shift;
    y >>= 1;
  end
endmodule
