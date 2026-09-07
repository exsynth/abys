module top
    (input logic [7 : 0] lhs[0 : 2],
     input logic [7 : 0] rhs[0 : 2],
     input logic condition,
     output logic [7 : 0] selected[0 : 2]);
  always_comb begin
    selected = condition ? lhs : rhs;
  end
endmodule
