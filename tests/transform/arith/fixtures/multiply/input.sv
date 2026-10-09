module top (
  input  logic [3:0] a,
  input  logic [3:0] b,
  input  logic [1:0] c,
  output logic [3:0] y,
  output logic [3:0] z
);
  assign y = a * b;
  assign z = a * c;
endmodule
