module top(
  input logic select,
  input logic [7:0] a,
  input logic [7:0] b,
  output logic [7:0] y
);
  always_comb begin
    if (select)
      y = a;
    else
      y = b;
  end
endmodule
