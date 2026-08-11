module top (
  input  logic       valid_i,
  output logic [0:0] valid_o
);
  always_comb begin
    valid_o = '0;
    valid_o[0] = valid_i;
  end
endmodule
