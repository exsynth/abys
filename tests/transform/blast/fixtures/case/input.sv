module top(
  input  logic [1:0] select,
  input  logic [3:0] a,
  input  logic [3:0] b,
  input  logic [3:0] c,
  output logic [3:0] y
);
  always_comb begin
    case (select)
      2'd0: y = a;
      2'd1: y = b;
      default: y = c;
    endcase
  end
endmodule
