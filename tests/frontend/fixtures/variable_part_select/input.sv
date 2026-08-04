module top(
  input logic [31:0] a,
  input logic [4:0] base,
  output logic [7:0] y
);
  assign y = a[base +: 8];
endmodule
