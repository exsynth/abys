module top (
  input [7:0] value_0_2,
  input [7:0] value_0_3,
  input [7:0] value_1_2,
  input [7:0] value_1_3,
  output  logic [31:0] result);

  logic [7:0] actual [0:1] [0:1];

function automatic [31:0] collect (
  input [7:0] formal [0:1] [0:1]
);
  begin
    logic [7:0] abys_dumper_tmp5 [0:1];
    logic signed [32:0] abys_dumper_tmp7;
    logic signed [32:0] abys_dumper_tmp9;
    logic [7:0] abys_dumper_tmp10;
    logic [7:0] abys_dumper_tmp12 [0:1];
    logic signed [32:0] abys_dumper_tmp14;
    logic signed [32:0] abys_dumper_tmp16;
    logic [7:0] abys_dumper_tmp17;
    logic [7:0] abys_dumper_tmp19 [0:1];
    logic signed [32:0] abys_dumper_tmp21;
    logic signed [32:0] abys_dumper_tmp23;
    logic [7:0] abys_dumper_tmp24;
    logic [7:0] abys_dumper_tmp26 [0:1];
    logic signed [32:0] abys_dumper_tmp28;
    logic signed [32:0] abys_dumper_tmp30;
    logic [7:0] abys_dumper_tmp31;
    logic [31:0] abys_dumper_tmp32;
    abys_dumper_tmp5 = formal[32'sb0];
    abys_dumper_tmp7 = 32'sb10;
    abys_dumper_tmp9 = (abys_dumper_tmp7 - 33'sb10);
    abys_dumper_tmp10 = abys_dumper_tmp5[abys_dumper_tmp9];
    abys_dumper_tmp12 = formal[32'sb0];
    abys_dumper_tmp14 = 32'sb11;
    abys_dumper_tmp16 = (abys_dumper_tmp14 - 33'sb10);
    abys_dumper_tmp17 = abys_dumper_tmp12[abys_dumper_tmp16];
    abys_dumper_tmp19 = formal[32'sb1];
    abys_dumper_tmp21 = 32'sb10;
    abys_dumper_tmp23 = (abys_dumper_tmp21 - 33'sb10);
    abys_dumper_tmp24 = abys_dumper_tmp19[abys_dumper_tmp23];
    abys_dumper_tmp26 = formal[32'sb1];
    abys_dumper_tmp28 = 32'sb11;
    abys_dumper_tmp30 = (abys_dumper_tmp28 - 33'sb10);
    abys_dumper_tmp31 = abys_dumper_tmp26[abys_dumper_tmp30];
    abys_dumper_tmp32 = {abys_dumper_tmp10, abys_dumper_tmp17, abys_dumper_tmp24, abys_dumper_tmp31};
    collect = abys_dumper_tmp32;
  end
endfunction


  always @(*)   begin
    logic [31:0] abys_dumper_tmp45;
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
    logic [7:0] abys_dumper_tmp44 [0:1] [0:1];
    abys_dumper_tmp38 = actual;
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
    abys_dumper_tmp42 = abys_dumper_tmp39[1'b1];
    abys_dumper_tmp43[0] = abys_dumper_tmp42[1];
    abys_dumper_tmp43[1] = abys_dumper_tmp42[0];
    abys_dumper_tmp44 = '{abys_dumper_tmp41, abys_dumper_tmp43};
    abys_dumper_tmp45 = collect(abys_dumper_tmp44);
    result = abys_dumper_tmp45;
    actual[32'sb0][abys_dumper_tmp6] = value_0_2;
    actual[32'sb0][abys_dumper_tmp16] = value_0_3;
    actual[32'sb1][abys_dumper_tmp25] = value_1_2;
    actual[32'sb1][abys_dumper_tmp34] = value_1_3;
  end
endmodule
