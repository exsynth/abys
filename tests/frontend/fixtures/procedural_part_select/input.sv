module top(
  input logic [7:0] a,
  input logic [3:0] upper,
  output logic [7:0] y
);
  always_comb begin
    y = a;
    y[7:4] = upper;
  end
endmodule
