module top(
  input  logic [3:0] a,
  input  logic [3:0] b,
  input  logic       select,
  output logic [3:0] y
);
  always_comb begin
    y = select ? a : b;
  end
endmodule
