module top(
  input logic [3:0] upper,
  input logic [3:0] lower,
  output logic [7:0] y
);
  assign y = {upper, lower};
endmodule
