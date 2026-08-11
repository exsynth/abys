module top (
  input valid_i,
  output  logic valid_o);



  always @(*)   begin
    valid_o = valid_i;
  end
endmodule
