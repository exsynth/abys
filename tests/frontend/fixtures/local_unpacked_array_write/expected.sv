module top (
  input [1:0] index,
  input [7:0] value,
  output  logic [7:0] y);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp42 [3:0];
    logic signed [32:0] abys_dumper_tmp10;
    logic signed [32:0] abys_dumper_tmp12;
    logic signed [32:0] abys_dumper_tmp17;
    logic signed [32:0] abys_dumper_tmp19;
    logic signed [32:0] abys_dumper_tmp24;
    logic signed [32:0] abys_dumper_tmp26;
    logic signed [32:0] abys_dumper_tmp31;
    logic signed [32:0] abys_dumper_tmp33;
    logic signed [3:0] abys_dumper_tmp38;
    logic signed [3:0] abys_dumper_tmp40;
    logic signed [32:0] abys_dumper_tmp44;
    logic signed [32:0] abys_dumper_tmp46;
    logic [7:0] abys_dumper_tmp47;
    abys_dumper_tmp42[0] = 8'bxxxxxxxx;
    abys_dumper_tmp42[1] = 8'bxxxxxxxx;
    abys_dumper_tmp42[2] = 8'bxxxxxxxx;
    abys_dumper_tmp42[3] = 8'bxxxxxxxx;
    abys_dumper_tmp10 = 32'sb0;
    abys_dumper_tmp12 = (33'sb11 - abys_dumper_tmp10);
    abys_dumper_tmp42[abys_dumper_tmp12] = 8'b10001;
    abys_dumper_tmp17 = 32'sb1;
    abys_dumper_tmp19 = (33'sb11 - abys_dumper_tmp17);
    abys_dumper_tmp42[abys_dumper_tmp19] = 8'b100010;
    abys_dumper_tmp24 = 32'sb10;
    abys_dumper_tmp26 = (33'sb11 - abys_dumper_tmp24);
    abys_dumper_tmp42[abys_dumper_tmp26] = 8'b110011;
    abys_dumper_tmp31 = 32'sb11;
    abys_dumper_tmp33 = (33'sb11 - abys_dumper_tmp31);
    abys_dumper_tmp42[abys_dumper_tmp33] = 8'b1000100;
    abys_dumper_tmp38 = index;
    abys_dumper_tmp40 = (4'sb11 - abys_dumper_tmp38);
    abys_dumper_tmp42[abys_dumper_tmp40] = value;
    abys_dumper_tmp44 = 32'sb10;
    abys_dumper_tmp46 = (33'sb11 - abys_dumper_tmp44);
    abys_dumper_tmp47 = abys_dumper_tmp42[abys_dumper_tmp46];
    y = abys_dumper_tmp47;
  end
endmodule
