module top(
  input logic [7:0] value4,
  input logic [7:0] value5,
  input logic [7:0] value6,
  input logic [7:0] value7,
  input logic [2:0] base,
  output logic [7:0] ascending_up_0,
  output logic [7:0] ascending_up_1,
  output logic [7:0] descending_down_0,
  output logic [7:0] descending_down_1,
  output logic [7:0] size_one
);
  logic [7:0] ascending [4:7];
  logic [7:0] descending [7:4];
  logic [7:0] ascending_up [0:1];
  logic [7:0] descending_down [0:1];
  logic [7:0] one [0:0];

  always_comb begin
    ascending[4] = value4;
    ascending[5] = value5;
    ascending[6] = value6;
    ascending[7] = value7;
    descending[4] = value4;
    descending[5] = value5;
    descending[6] = value6;
    descending[7] = value7;

    ascending_up = ascending[base +: 2];
    descending_down = descending[base -: 2];
    one = descending[base +: 1];

    ascending_up_0 = ascending_up[0];
    ascending_up_1 = ascending_up[1];
    descending_down_0 = descending_down[0];
    descending_down_1 = descending_down[1];
    size_one = one[0];
  end
endmodule
