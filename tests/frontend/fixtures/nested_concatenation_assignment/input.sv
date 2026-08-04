module top(
  input  logic [7:0] a,
  output logic [1:0] first,
  output logic [3:0] middle,
  output logic [1:0] last
);
  always_comb begin
    {first, {middle, last}} = a;
  end
endmodule
