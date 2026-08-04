module top(
  input logic signed [7:0] a,
  input logic [2:0] amount,
  output logic signed [7:0] y
);
  assign y = a >>> amount;
endmodule
