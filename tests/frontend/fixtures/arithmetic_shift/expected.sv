module top (
  input signed [7:0] a,
  input [2:0] amount,
  output  logic signed [7:0] y);



  always @(*)   begin
    logic signed [7:0] abys_dumper_tmp4;
    abys_dumper_tmp4 = ($signed(a) >>> amount);
    y = abys_dumper_tmp4;
  end
endmodule
