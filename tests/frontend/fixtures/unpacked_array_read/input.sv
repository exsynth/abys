module top(
  input logic [7:0] a [0:3],
  input logic [1:0] index,
  output logic [7:0] y
);
  assign y = a[index];
endmodule
