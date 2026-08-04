module top (
  input [1:0] index,
  input [7:0] value,
  output  logic [7:0] y);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp37 [3:0];
    logic signed [32:0] abys_dumper_tmp4;
    logic signed [32:0] abys_dumper_tmp6;
    logic signed [32:0] abys_dumper_tmp12;
    logic signed [32:0] abys_dumper_tmp14;
    logic signed [32:0] abys_dumper_tmp19;
    logic signed [32:0] abys_dumper_tmp21;
    logic signed [32:0] abys_dumper_tmp26;
    logic signed [32:0] abys_dumper_tmp28;
    logic signed [3:0] abys_dumper_tmp33;
    logic signed [3:0] abys_dumper_tmp35;
    logic signed [32:0] abys_dumper_tmp39;
    logic signed [32:0] abys_dumper_tmp41;
    logic [7:0] abys_dumper_tmp42;
    abys_dumper_tmp4 = 32'sb0;
    abys_dumper_tmp6 = (33'sb11 - abys_dumper_tmp4);
    abys_dumper_tmp37[abys_dumper_tmp6] = 8'b10001;
    abys_dumper_tmp12 = 32'sb1;
    abys_dumper_tmp14 = (33'sb11 - abys_dumper_tmp12);
    abys_dumper_tmp37[abys_dumper_tmp14] = 8'b100010;
    abys_dumper_tmp19 = 32'sb10;
    abys_dumper_tmp21 = (33'sb11 - abys_dumper_tmp19);
    abys_dumper_tmp37[abys_dumper_tmp21] = 8'b110011;
    abys_dumper_tmp26 = 32'sb11;
    abys_dumper_tmp28 = (33'sb11 - abys_dumper_tmp26);
    abys_dumper_tmp37[abys_dumper_tmp28] = 8'b1000100;
    abys_dumper_tmp33 = index;
    abys_dumper_tmp35 = (4'sb11 - abys_dumper_tmp33);
    abys_dumper_tmp37[abys_dumper_tmp35] = value;
    abys_dumper_tmp39 = 32'sb10;
    abys_dumper_tmp41 = (33'sb11 - abys_dumper_tmp39);
    abys_dumper_tmp42 = abys_dumper_tmp37[abys_dumper_tmp41];
    y = abys_dumper_tmp42;
  end
endmodule
