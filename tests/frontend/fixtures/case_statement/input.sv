module top(
  input logic [1:0] select,
  input logic [7:0] a,
  input logic [7:0] b,
  output logic [7:0] y
);
  always_comb begin
    case (select)
      2'b00: y = a;
      2'b01: y = b;
      default: y = '0;
    endcase
  end
endmodule
