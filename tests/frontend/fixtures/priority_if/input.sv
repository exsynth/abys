module top(
  input logic first,
  input logic second,
  input logic [7:0] a,
  input logic [7:0] b,
  input logic [7:0] c,
  output logic [7:0] y
);
  always_comb begin
    if (first)
      y = a;
    else if (second)
      y = b;
    else
      y = c;
  end
endmodule
