module top (
  input [7:0] value_0_2,
  input [7:0] value_0_3,
  input [7:0] value_1_2,
  input [7:0] value_1_3,
  output  logic [7:0] result_0_2,
  output  logic [7:0] result_0_3,
  output  logic [7:0] result_1_2,
  output  logic [7:0] result_1_3);

  logic [7:0] source [0:1] [0:1];
  logic [7:0] target [0:1] [0:1];


  always @(*)   begin
    logic [7:0] abys_dumper_tmp46 [0:1] [0:1];
    logic [7:0] abys_dumper_tmp38 [0:1] [0:1];
    logic signed [32:0] abys_dumper_tmp4;
    logic signed [32:0] abys_dumper_tmp6;
    logic signed [32:0] abys_dumper_tmp14;
    logic signed [32:0] abys_dumper_tmp16;
    logic signed [32:0] abys_dumper_tmp23;
    logic signed [32:0] abys_dumper_tmp25;
    logic signed [32:0] abys_dumper_tmp32;
    logic signed [32:0] abys_dumper_tmp34;
    logic [7:0] abys_dumper_tmp39 [0:1] [0:1];
    logic [7:0] abys_dumper_tmp40 [0:1];
    logic [7:0] abys_dumper_tmp41 [0:1];
    logic [7:0] abys_dumper_tmp42 [0:1];
    logic [7:0] abys_dumper_tmp43 [0:1];
    logic [7:0] abys_dumper_tmp69 [0:1];
    logic signed [32:0] abys_dumper_tmp71;
    logic signed [32:0] abys_dumper_tmp73;
    logic [7:0] abys_dumper_tmp74;
    logic [7:0] abys_dumper_tmp62 [0:1];
    logic signed [32:0] abys_dumper_tmp64;
    logic signed [32:0] abys_dumper_tmp66;
    logic [7:0] abys_dumper_tmp67;
    logic [7:0] abys_dumper_tmp55 [0:1];
    logic signed [32:0] abys_dumper_tmp57;
    logic signed [32:0] abys_dumper_tmp59;
    logic [7:0] abys_dumper_tmp60;
    logic [7:0] abys_dumper_tmp48 [0:1];
    logic signed [32:0] abys_dumper_tmp50;
    logic signed [32:0] abys_dumper_tmp52;
    logic [7:0] abys_dumper_tmp53;
    abys_dumper_tmp46 = target;
    abys_dumper_tmp38 = source;
    abys_dumper_tmp4 = 32'sb10;
    abys_dumper_tmp6 = (abys_dumper_tmp4 - 33'sb10);
    abys_dumper_tmp38[32'sb0][abys_dumper_tmp6] = value_0_2;
    abys_dumper_tmp14 = 32'sb11;
    abys_dumper_tmp16 = (abys_dumper_tmp14 - 33'sb10);
    abys_dumper_tmp38[32'sb0][abys_dumper_tmp16] = value_0_3;
    abys_dumper_tmp23 = 32'sb10;
    abys_dumper_tmp25 = (abys_dumper_tmp23 - 33'sb10);
    abys_dumper_tmp38[32'sb1][abys_dumper_tmp25] = value_1_2;
    abys_dumper_tmp32 = 32'sb11;
    abys_dumper_tmp34 = (abys_dumper_tmp32 - 33'sb10);
    abys_dumper_tmp38[32'sb1][abys_dumper_tmp34] = value_1_3;
    abys_dumper_tmp39[0] = abys_dumper_tmp38[1];
    abys_dumper_tmp39[1] = abys_dumper_tmp38[0];
    abys_dumper_tmp40 = abys_dumper_tmp39[1'b0];
    abys_dumper_tmp41[0] = abys_dumper_tmp40[1];
    abys_dumper_tmp41[1] = abys_dumper_tmp40[0];
    abys_dumper_tmp46[0] = abys_dumper_tmp41;
    abys_dumper_tmp42 = abys_dumper_tmp39[1'b1];
    abys_dumper_tmp43[0] = abys_dumper_tmp42[1];
    abys_dumper_tmp43[1] = abys_dumper_tmp42[0];
    abys_dumper_tmp46[1] = abys_dumper_tmp43;
    abys_dumper_tmp69 = abys_dumper_tmp46[32'sb1];
    abys_dumper_tmp71 = 32'sb11;
    abys_dumper_tmp73 = (abys_dumper_tmp71 - 33'sb10);
    abys_dumper_tmp74 = abys_dumper_tmp69[abys_dumper_tmp73];
    abys_dumper_tmp62 = abys_dumper_tmp46[32'sb1];
    abys_dumper_tmp64 = 32'sb10;
    abys_dumper_tmp66 = (abys_dumper_tmp64 - 33'sb10);
    abys_dumper_tmp67 = abys_dumper_tmp62[abys_dumper_tmp66];
    abys_dumper_tmp55 = abys_dumper_tmp46[32'sb0];
    abys_dumper_tmp57 = 32'sb11;
    abys_dumper_tmp59 = (abys_dumper_tmp57 - 33'sb10);
    abys_dumper_tmp60 = abys_dumper_tmp55[abys_dumper_tmp59];
    abys_dumper_tmp48 = abys_dumper_tmp46[32'sb0];
    abys_dumper_tmp50 = 32'sb10;
    abys_dumper_tmp52 = (abys_dumper_tmp50 - 33'sb10);
    abys_dumper_tmp53 = abys_dumper_tmp48[abys_dumper_tmp52];
    result_1_3 = abys_dumper_tmp74;
    result_1_2 = abys_dumper_tmp67;
    result_0_3 = abys_dumper_tmp60;
    result_0_2 = abys_dumper_tmp53;
    target[0] = abys_dumper_tmp41;
    target[1] = abys_dumper_tmp43;
    source[32'sb0][abys_dumper_tmp6] = value_0_2;
    source[32'sb0][abys_dumper_tmp16] = value_0_3;
    source[32'sb1][abys_dumper_tmp25] = value_1_2;
    source[32'sb1][abys_dumper_tmp34] = value_1_3;
  end
endmodule
