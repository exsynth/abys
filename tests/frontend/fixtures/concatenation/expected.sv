module top (
  input [3:0] upper,
  input [3:0] lower,
  output  logic [7:0] y);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp4;
    abys_dumper_tmp4 = {upper, lower};
    y = abys_dumper_tmp4;
  end
endmodule
