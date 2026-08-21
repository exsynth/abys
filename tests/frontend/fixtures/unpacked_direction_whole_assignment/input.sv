module top(
  input logic [7:0] value_0_2,
  input logic [7:0] value_0_3,
  input logic [7:0] value_1_2,
  input logic [7:0] value_1_3,
  output logic [7:0] result_0_2,
  output logic [7:0] result_0_3,
  output logic [7:0] result_1_2,
  output logic [7:0] result_1_3
);
  logic [7:0] source [0:1][3:2];
  logic [7:0] target [1:0][2:3];

  always_comb begin
    source[0][2] = value_0_2;
    source[0][3] = value_0_3;
    source[1][2] = value_1_2;
    source[1][3] = value_1_3;
    target = source;
    result_0_2 = target[0][2];
    result_0_3 = target[0][3];
    result_1_2 = target[1][2];
    result_1_3 = target[1][3];
  end
endmodule
