module top(
  input logic [7:0] a,
  output logic y
);
  assign y = ^a;
endmodule
