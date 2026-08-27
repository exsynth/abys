module top(
  input  logic [1:0] b,
  input  logic       enable,
  output logic [1:0] q
);
  always_latch begin
    if (enable) begin
      q[0] = b[0];
    end
    q[1] = b[1];
  end
endmodule
