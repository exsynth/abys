module top (
  input [15:0] d_i,
  output  logic [15:0] d_o);



  always @(*)   begin
    logic [15:0] abys_dumper_tmp3;
    abys_dumper_tmp3 = {d_i};
    d_o = abys_dumper_tmp3;
  end
endmodule
