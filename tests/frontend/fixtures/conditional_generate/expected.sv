module top (
  input [3:0] d_i,
  output  logic [3:0] d_o);



  always @(*)   begin
    d_o = d_i;
  end
endmodule
