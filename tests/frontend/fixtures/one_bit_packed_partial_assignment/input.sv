module top (
  input  logic       valid_i,
  input  logic       index_i,
  output logic [0:0] valid_o
);
  always_comb begin
    valid_o = '0;
    valid_o[index_i] = valid_i;
  end
endmodule
