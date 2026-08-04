module top(
  input  logic [-4:3] negative_range,
  input  logic signed [3:0] negative_index,
  input  logic [1000:993] offset_range,
  input  logic [9:0] offset_index,
  output logic negative_y,
  output logic offset_y
);
  assign negative_y = negative_range[negative_index];
  assign offset_y = offset_range[offset_index];
endmodule
