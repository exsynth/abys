module top(
  input  logic [3:0] upper,
  input  logic [3:0] lower,
  output logic [7:0] y
);
  always_comb begin
    logic [7:0] temporary;
    temporary[7:4] = upper;
    temporary[3:0] = lower;
    y = temporary;
  end
endmodule
