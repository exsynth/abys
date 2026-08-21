module top (
  input [7:0] value4,
  input [7:0] value5,
  input [7:0] value6,
  input [7:0] value7,
  input [2:0] index,
  output  logic [7:0] ascending_value,
  output  logic [7:0] descending_value,
  output  logic [7:0] positional_value);

  logic [7:0] ascending [0:3];
  logic [7:0] descending [0:3];
  logic [7:0] positional_copy [0:3];


  always @(*)   begin
    logic [7:0] abys_dumper_tmp58 [0:3];
    logic [7:0] abys_dumper_tmp55 [0:3];
    logic signed [32:0] abys_dumper_tmp32;
    logic signed [32:0] abys_dumper_tmp34;
    logic signed [32:0] abys_dumper_tmp39;
    logic signed [32:0] abys_dumper_tmp41;
    logic signed [32:0] abys_dumper_tmp45;
    logic signed [32:0] abys_dumper_tmp47;
    logic signed [32:0] abys_dumper_tmp51;
    logic signed [32:0] abys_dumper_tmp53;
    logic [7:0] abys_dumper_tmp56 [0:3];
    logic signed [32:0] abys_dumper_tmp69;
    logic signed [32:0] abys_dumper_tmp71;
    logic [7:0] abys_dumper_tmp72;
    logic signed [4:0] abys_dumper_tmp64;
    logic signed [4:0] abys_dumper_tmp66;
    logic [7:0] abys_dumper_tmp67;
    logic [7:0] abys_dumper_tmp30 [0:3];
    logic signed [32:0] abys_dumper_tmp4;
    logic signed [32:0] abys_dumper_tmp6;
    logic signed [32:0] abys_dumper_tmp12;
    logic signed [32:0] abys_dumper_tmp14;
    logic signed [32:0] abys_dumper_tmp19;
    logic signed [32:0] abys_dumper_tmp21;
    logic signed [32:0] abys_dumper_tmp26;
    logic signed [32:0] abys_dumper_tmp28;
    logic signed [4:0] abys_dumper_tmp60;
    logic signed [4:0] abys_dumper_tmp62;
    logic [7:0] abys_dumper_tmp63;
    abys_dumper_tmp58 = positional_copy;
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
    abys_dumper_tmp56[0] = abys_dumper_tmp55[3];
    abys_dumper_tmp56[1] = abys_dumper_tmp55[2];
    abys_dumper_tmp56[2] = abys_dumper_tmp55[1];
    abys_dumper_tmp56[3] = abys_dumper_tmp55[0];
    abys_dumper_tmp58 = abys_dumper_tmp56;
    abys_dumper_tmp69 = 32'sb100;
    abys_dumper_tmp71 = (abys_dumper_tmp69 - 33'sb100);
    abys_dumper_tmp72 = abys_dumper_tmp58[abys_dumper_tmp71];
    abys_dumper_tmp64 = index;
    abys_dumper_tmp66 = (abys_dumper_tmp64 - 5'sb100);
    abys_dumper_tmp67 = abys_dumper_tmp55[abys_dumper_tmp66];
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
    abys_dumper_tmp60 = index;
    abys_dumper_tmp62 = (abys_dumper_tmp60 - 5'sb100);
    abys_dumper_tmp63 = abys_dumper_tmp30[abys_dumper_tmp62];
    positional_value = abys_dumper_tmp72;
    descending_value = abys_dumper_tmp67;
    descending[abys_dumper_tmp34] = value4;
    descending[abys_dumper_tmp41] = value5;
    descending[abys_dumper_tmp47] = value6;
    descending[abys_dumper_tmp53] = value7;
    ascending_value = abys_dumper_tmp63;
    positional_copy = abys_dumper_tmp56;
    ascending[abys_dumper_tmp6] = value4;
    ascending[abys_dumper_tmp14] = value5;
    ascending[abys_dumper_tmp21] = value6;
    ascending[abys_dumper_tmp28] = value7;
  end
endmodule
