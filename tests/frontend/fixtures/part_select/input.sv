module top(
  input logic [15:0] a,
  output logic [7:0] y
);
  assign y = a[11:4];
endmodule
