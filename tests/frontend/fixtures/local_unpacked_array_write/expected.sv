module top (
  input [1:0] index,
  input [7:0] value,
  output  logic [7:0] y);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp41 [3:0];
    logic [7:0] abys_dumper_tmp6 [3:0];
    logic signed [32:0] abys_dumper_tmp9;
    logic signed [32:0] abys_dumper_tmp11;
    logic signed [32:0] abys_dumper_tmp16;
    logic signed [32:0] abys_dumper_tmp18;
    logic signed [32:0] abys_dumper_tmp23;
    logic signed [32:0] abys_dumper_tmp25;
    logic signed [32:0] abys_dumper_tmp30;
    logic signed [32:0] abys_dumper_tmp32;
    logic signed [3:0] abys_dumper_tmp37;
    logic signed [3:0] abys_dumper_tmp39;
    logic signed [32:0] abys_dumper_tmp43;
    logic signed [32:0] abys_dumper_tmp45;
    logic [7:0] abys_dumper_tmp46;
    abys_dumper_tmp6 = '{8'bxxxxxxxx, 8'bxxxxxxxx, 8'bxxxxxxxx, 8'bxxxxxxxx};
    abys_dumper_tmp41 = abys_dumper_tmp6;
    abys_dumper_tmp9 = 32'sb0;
    abys_dumper_tmp11 = (33'sb11 - abys_dumper_tmp9);
    abys_dumper_tmp41[abys_dumper_tmp11] = 8'b10001;
    abys_dumper_tmp16 = 32'sb1;
    abys_dumper_tmp18 = (33'sb11 - abys_dumper_tmp16);
    abys_dumper_tmp41[abys_dumper_tmp18] = 8'b100010;
    abys_dumper_tmp23 = 32'sb10;
    abys_dumper_tmp25 = (33'sb11 - abys_dumper_tmp23);
    abys_dumper_tmp41[abys_dumper_tmp25] = 8'b110011;
    abys_dumper_tmp30 = 32'sb11;
    abys_dumper_tmp32 = (33'sb11 - abys_dumper_tmp30);
    abys_dumper_tmp41[abys_dumper_tmp32] = 8'b1000100;
    abys_dumper_tmp37 = index;
    abys_dumper_tmp39 = (4'sb11 - abys_dumper_tmp37);
    abys_dumper_tmp41[abys_dumper_tmp39] = value;
    abys_dumper_tmp43 = 32'sb10;
    abys_dumper_tmp45 = (33'sb11 - abys_dumper_tmp43);
    abys_dumper_tmp46 = abys_dumper_tmp41[abys_dumper_tmp45];
    y = abys_dumper_tmp46;
  end
endmodule
