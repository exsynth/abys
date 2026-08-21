module top(
  input logic [7:0] value_2_6,
  input logic [7:0] value_2_7,
  input logic [7:0] value_3_6,
  input logic [7:0] value_3_7,
  input logic [1:0] row,
  input logic [2:0] column,
  output logic [7:0] ascending_descending_value,
  output logic [7:0] descending_ascending_value
);
  logic [7:0] ascending_descending [2:3][7:6];
  logic [7:0] descending_ascending [3:2][6:7];

  always_comb begin
    ascending_descending[2][6] = value_2_6;
    ascending_descending[2][7] = value_2_7;
    ascending_descending[3][6] = value_3_6;
    ascending_descending[3][7] = value_3_7;

    descending_ascending[2][6] = value_2_6;
    descending_ascending[2][7] = value_2_7;
    descending_ascending[3][6] = value_3_6;
    descending_ascending[3][7] = value_3_7;

    ascending_descending_value = ascending_descending[row][column];
    descending_ascending_value = descending_ascending[row][column];
  end
endmodule
