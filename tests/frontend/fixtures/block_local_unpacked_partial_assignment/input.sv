module top (
  input  logic [3:0] d_i,
  output logic [3:0] d_o
);
  always_comb begin
    logic [3:0] temporary [0:1];
    temporary[0] = d_i;
    d_o = temporary[1];
  end
endmodule
