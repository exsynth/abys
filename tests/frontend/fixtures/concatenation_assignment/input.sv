module top(
  input  logic [7:0] a,
  output logic [3:0] upper,
  output logic [3:0] lower
);
  always_comb begin
    {upper, lower} = a;
  end
endmodule
