module top(
  input logic [7:0] value3,
  input logic [7:0] value2,
  input logic [7:0] value1,
  input logic [7:0] value0,
  output logic [7:0] result3,
  output logic [7:0] result2,
  output logic [7:0] result1,
  output logic [7:0] result0
);
  logic [7:0] values [3:0];

  always_comb begin
    values = '{value3, value2, value1, value0};
    result3 = values[3];
    result2 = values[2];
    result1 = values[1];
    result0 = values[0];
  end
endmodule
