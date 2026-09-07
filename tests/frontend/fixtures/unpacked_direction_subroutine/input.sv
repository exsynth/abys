module top(
  input logic [7:0] value_0_2,
  input logic [7:0] value_0_3,
  input logic [7:0] value_1_2,
  input logic [7:0] value_1_3,
  output logic [31:0] result
);
  logic [7:0] actual [0:1][3:2];

  function automatic logic [31:0] collect(input logic [7:0] formal [1:0][2:3]);
    return {formal[0][2], formal[0][3], formal[1][2], formal[1][3]};
  endfunction

  always_comb begin
    actual[0][2] = value_0_2;
    actual[0][3] = value_0_3;
    actual[1][2] = value_1_2;
    actual[1][3] = value_1_3;
    result = collect(actual);
  end
endmodule
