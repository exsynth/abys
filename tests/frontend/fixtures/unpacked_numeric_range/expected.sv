module top (
  input [7:0] value4,
  input [7:0] value5,
  input [7:0] value6,
  input [7:0] value7,
  input [2:0] base,
  output  logic [7:0] ascending_up_0,
  output  logic [7:0] ascending_up_1,
  output  logic [7:0] descending_down_0,
  output  logic [7:0] descending_down_1,
  output  logic [7:0] size_one);

  logic [7:0] ascending [0:3];
  logic [7:0] descending [0:3];
  logic [7:0] ascending_up [0:1];
  logic [7:0] descending_down [0:1];
  logic [7:0] one [0:0];


  always @(*)   begin
    logic [7:0] abys_dumper_tmp62 [0:1];
    logic [7:0] abys_dumper_tmp30 [0:3];
    logic signed [32:0] abys_dumper_tmp4;
    logic signed [32:0] abys_dumper_tmp6;
    logic signed [32:0] abys_dumper_tmp12;
    logic signed [32:0] abys_dumper_tmp14;
    logic signed [32:0] abys_dumper_tmp19;
    logic signed [32:0] abys_dumper_tmp21;
    logic signed [32:0] abys_dumper_tmp26;
    logic signed [32:0] abys_dumper_tmp28;
    logic signed [4:0] abys_dumper_tmp57;
    logic signed [4:0] abys_dumper_tmp59;
    logic [7:0] abys_dumper_tmp60 [0:1];
    logic [7:0] abys_dumper_tmp81;
    logic [7:0] abys_dumper_tmp79;
    logic [7:0] abys_dumper_tmp71 [0:1];
    logic [7:0] abys_dumper_tmp55 [0:3];
    logic signed [32:0] abys_dumper_tmp32;
    logic signed [32:0] abys_dumper_tmp34;
    logic signed [32:0] abys_dumper_tmp39;
    logic signed [32:0] abys_dumper_tmp41;
    logic signed [32:0] abys_dumper_tmp45;
    logic signed [32:0] abys_dumper_tmp47;
    logic signed [32:0] abys_dumper_tmp51;
    logic signed [32:0] abys_dumper_tmp53;
    logic signed [5:0] abys_dumper_tmp63;
    logic signed [5:0] abys_dumper_tmp65;
    logic signed [5:0] abys_dumper_tmp67;
    logic [7:0] abys_dumper_tmp68 [0:1];
    logic [7:0] abys_dumper_tmp69 [0:1];
    logic [7:0] abys_dumper_tmp85;
    logic signed [4:0] abys_dumper_tmp72;
    logic signed [4:0] abys_dumper_tmp74;
    logic [7:0] abys_dumper_tmp75 [0:0];
    logic [7:0] abys_dumper_tmp77 [0:0];
    logic [7:0] abys_dumper_tmp87;
    logic [7:0] abys_dumper_tmp83;
    abys_dumper_tmp62 = ascending_up;
    abys_dumper_tmp30 = ascending;
    abys_dumper_tmp4 = 32'sb100;
    abys_dumper_tmp6 = (abys_dumper_tmp4 - 33'sb100);
    abys_dumper_tmp30[abys_dumper_tmp6] = value4;
    abys_dumper_tmp12 = 32'sb101;
    abys_dumper_tmp14 = (abys_dumper_tmp12 - 33'sb100);
    abys_dumper_tmp30[abys_dumper_tmp14] = value5;
    abys_dumper_tmp19 = 32'sb110;
    abys_dumper_tmp21 = (abys_dumper_tmp19 - 33'sb100);
    abys_dumper_tmp30[abys_dumper_tmp21] = value6;
    abys_dumper_tmp26 = 32'sb111;
    abys_dumper_tmp28 = (abys_dumper_tmp26 - 33'sb100);
    abys_dumper_tmp30[abys_dumper_tmp28] = value7;
    abys_dumper_tmp57 = base;
    abys_dumper_tmp59 = (abys_dumper_tmp57 - 5'sb100);
    abys_dumper_tmp60 = abys_dumper_tmp30[abys_dumper_tmp59 +: 2];
    abys_dumper_tmp62 = abys_dumper_tmp60;
    abys_dumper_tmp81 = abys_dumper_tmp62[32'sb1];
    abys_dumper_tmp79 = abys_dumper_tmp62[32'sb0];
    abys_dumper_tmp71 = descending_down;
    abys_dumper_tmp55 = descending;
    abys_dumper_tmp32 = 32'sb100;
    abys_dumper_tmp34 = (abys_dumper_tmp32 - 33'sb100);
    abys_dumper_tmp55[abys_dumper_tmp34] = value4;
    abys_dumper_tmp39 = 32'sb101;
    abys_dumper_tmp41 = (abys_dumper_tmp39 - 33'sb100);
    abys_dumper_tmp55[abys_dumper_tmp41] = value5;
    abys_dumper_tmp45 = 32'sb110;
    abys_dumper_tmp47 = (abys_dumper_tmp45 - 33'sb100);
    abys_dumper_tmp55[abys_dumper_tmp47] = value6;
    abys_dumper_tmp51 = 32'sb111;
    abys_dumper_tmp53 = (abys_dumper_tmp51 - 33'sb100);
    abys_dumper_tmp55[abys_dumper_tmp53] = value7;
    abys_dumper_tmp63 = base;
    abys_dumper_tmp65 = (abys_dumper_tmp63 + -6'sb1);
    abys_dumper_tmp67 = (abys_dumper_tmp65 - 6'sb100);
    abys_dumper_tmp68 = abys_dumper_tmp55[abys_dumper_tmp67 +: 2];
    abys_dumper_tmp69[0] = abys_dumper_tmp68[1];
    abys_dumper_tmp69[1] = abys_dumper_tmp68[0];
    abys_dumper_tmp71 = abys_dumper_tmp69;
    abys_dumper_tmp85 = abys_dumper_tmp71[32'sb1];
    abys_dumper_tmp72 = base;
    abys_dumper_tmp74 = (abys_dumper_tmp72 - 5'sb100);
    abys_dumper_tmp75 = abys_dumper_tmp55[abys_dumper_tmp74 +: 1];
    abys_dumper_tmp77 = one;
    abys_dumper_tmp77 = abys_dumper_tmp75;
    abys_dumper_tmp87 = abys_dumper_tmp77[32'sb0];
    abys_dumper_tmp83 = abys_dumper_tmp71[32'sb0];
    ascending_up_1 = abys_dumper_tmp81;
    ascending_up_0 = abys_dumper_tmp79;
    descending_down_1 = abys_dumper_tmp85;
    one = abys_dumper_tmp75;
    descending_down = abys_dumper_tmp69;
    ascending_up = abys_dumper_tmp60;
    size_one = abys_dumper_tmp87;
    descending[abys_dumper_tmp34] = value4;
    descending[abys_dumper_tmp41] = value5;
    descending[abys_dumper_tmp47] = value6;
    descending[abys_dumper_tmp53] = value7;
    descending_down_0 = abys_dumper_tmp83;
    ascending[abys_dumper_tmp6] = value4;
    ascending[abys_dumper_tmp14] = value5;
    ascending[abys_dumper_tmp21] = value6;
    ascending[abys_dumper_tmp28] = value7;
  end
endmodule
