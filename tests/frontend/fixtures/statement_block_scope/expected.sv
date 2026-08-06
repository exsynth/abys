module top (
  output  logic a,
  output  logic c);

  logic b_abys_zero_scope;
  logic b_abys_one_scope;


  always @(*)   begin
    b_abys_zero_scope = 1'b0;
  end
  always @(*)   begin
    a = b_abys_zero_scope;
  end
  always @(*)   begin
    b_abys_one_scope = 1'b1;
  end
  always @(*)   begin
    c = b_abys_one_scope;
  end
endmodule
