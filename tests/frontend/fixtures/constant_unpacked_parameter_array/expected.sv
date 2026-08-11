module top (
  input [1:0] select_i,
  output  logic [7:0] d_o);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp6 [3:0];
    logic signed [3:0] abys_dumper_tmp8;
    logic signed [3:0] abys_dumper_tmp10;
    logic [7:0] abys_dumper_tmp11;
    abys_dumper_tmp6 = '{8'b10010, 8'b110100, 8'b1010110, 8'b1111000};
    abys_dumper_tmp8 = select_i;
    abys_dumper_tmp10 = (4'sb11 - abys_dumper_tmp8);
    abys_dumper_tmp11 = abys_dumper_tmp6[abys_dumper_tmp10];
    d_o = abys_dumper_tmp11;
  end
endmodule
