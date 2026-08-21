module top(
  input logic [7:0] value4,
  input logic [7:0] value5,
  input logic [7:0] value6,
  input logic [7:0] value7,
  input logic [2:0] index,
  output logic [7:0] ascending_value,
  output logic [7:0] descending_value,
  output logic [7:0] positional_value
);
  logic [7:0] ascending [4:7];
  logic [7:0] descending [7:4];
  logic [7:0] positional_copy [4:7];

  always_comb begin
    ascending[4] = value4;
    ascending[5] = value5;
    ascending[6] = value6;
    ascending[7] = value7;
    descending[4] = value4;
    descending[5] = value5;
    descending[6] = value6;
    descending[7] = value7;
    positional_copy = descending;
    ascending_value = ascending[index];
    descending_value = descending[index];
    positional_value = positional_copy[4];
  end
endmodule
