module top (
  input  logic [3:0] d_i,
  output logic [7:0] d_o
);
  always_comb begin
    logic [7:0] temporary;
    temporary[7:4] = d_i;
    d_o = temporary;
  end
endmodule
