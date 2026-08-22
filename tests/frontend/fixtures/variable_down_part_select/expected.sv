module top (
  input [31:0] a,
  input [4:0] base,
  output  logic [7:0] y);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp6;
    abys_dumper_tmp6 = a[($signed(10'($signed(-4'sb111))) + ($signed(10'({1'b0, base})) * $signed(10'($signed(2'sb1))))) +: 8];
    y = abys_dumper_tmp6;
  end
endmodule
