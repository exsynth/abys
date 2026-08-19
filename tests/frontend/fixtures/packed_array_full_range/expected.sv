module top (
  input [23:0] data_i,
  output  logic [23:0] simple_o,
  output  logic [23:0] indexed_o);



  always @(*)   begin
    simple_o = data_i;
  end
  always @(*)   begin
    indexed_o = data_i;
  end
endmodule
