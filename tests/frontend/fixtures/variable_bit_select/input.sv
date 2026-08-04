module top(
  input logic [15:0] a,
  input logic [3:0] index,
  output logic y
);
  assign y = a[index];
endmodule
