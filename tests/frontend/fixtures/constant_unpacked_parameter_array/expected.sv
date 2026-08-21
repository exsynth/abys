module top (
  input [1:0] select_i,
  output  logic [7:0] d_o);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp6 [0:3];
    logic [7:0] abys_dumper_tmp8;
    abys_dumper_tmp6 = '{8'b10010, 8'b110100, 8'b1010110, 8'b1111000};
    abys_dumper_tmp8 = abys_dumper_tmp6[select_i];
    d_o = abys_dumper_tmp8;
  end
endmodule
