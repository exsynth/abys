module top(
    input  logic [3:0] a,
    input  logic [3:0] b,
    input  logic       select,
    output logic       y
);
  assign y = select ? (a + b == 4'd7) : (a == b);
endmodule
