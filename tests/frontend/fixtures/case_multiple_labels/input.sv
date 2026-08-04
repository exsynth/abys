module top(
  input  logic [2:0] selector,
  input  logic [7:0] a,
  input  logic [7:0] b,
  output logic [7:0] y
);
  always_comb begin
    case (selector)
      3'd0, 3'd2: y = a;
      3'd1, 3'd3: y = b;
      default: y = a ^ b;
    endcase
  end
endmodule
