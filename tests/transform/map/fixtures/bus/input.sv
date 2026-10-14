module top (
  input  logic [3:0] a,
  input  logic [3:0] b,
  output logic [3:0] y
);
  assign y = a & b;
endmodule
