module top (
  input [7:0] value_2_6,
  input [7:0] value_2_7,
  input [7:0] value_3_6,
  input [7:0] value_3_7,
  input [1:0] row,
  input [2:0] column,
  output  logic [7:0] ascending_descending_value,
  output  logic [7:0] descending_ascending_value);

  logic [7:0] ascending_descending [0:1] [0:1];
  logic [7:0] descending_ascending [0:1] [0:1];


  always @(*)   begin
    logic [7:0] abys_dumper_tmp50 [0:1] [0:1];
    logic signed [32:0] abys_dumper_tmp9;
    logic signed [32:0] abys_dumper_tmp11;
    logic signed [32:0] abys_dumper_tmp4;
    logic signed [32:0] abys_dumper_tmp6;
    logic signed [32:0] abys_dumper_tmp22;
    logic signed [32:0] abys_dumper_tmp24;
    logic signed [32:0] abys_dumper_tmp17;
    logic signed [32:0] abys_dumper_tmp19;
    logic signed [32:0] abys_dumper_tmp34;
    logic signed [32:0] abys_dumper_tmp36;
    logic signed [32:0] abys_dumper_tmp29;
    logic signed [32:0] abys_dumper_tmp31;
    logic signed [32:0] abys_dumper_tmp46;
    logic signed [32:0] abys_dumper_tmp48;
    logic signed [32:0] abys_dumper_tmp41;
    logic signed [32:0] abys_dumper_tmp43;
    logic signed [3:0] abys_dumper_tmp97;
    logic signed [3:0] abys_dumper_tmp99;
    logic [7:0] abys_dumper_tmp100 [0:1];
    logic signed [4:0] abys_dumper_tmp102;
    logic signed [4:0] abys_dumper_tmp104;
    logic [7:0] abys_dumper_tmp105;
    logic signed [32:0] abys_dumper_tmp57;
    logic signed [32:0] abys_dumper_tmp59;
    logic signed [32:0] abys_dumper_tmp52;
    logic signed [32:0] abys_dumper_tmp54;
    logic signed [32:0] abys_dumper_tmp69;
    logic signed [32:0] abys_dumper_tmp71;
    logic signed [32:0] abys_dumper_tmp64;
    logic signed [32:0] abys_dumper_tmp66;
    logic signed [32:0] abys_dumper_tmp80;
    logic signed [32:0] abys_dumper_tmp82;
    logic signed [32:0] abys_dumper_tmp75;
    logic signed [32:0] abys_dumper_tmp77;
    logic signed [32:0] abys_dumper_tmp91;
    logic signed [32:0] abys_dumper_tmp93;
    logic signed [32:0] abys_dumper_tmp86;
    logic signed [32:0] abys_dumper_tmp88;
    logic [7:0] abys_dumper_tmp95 [0:1] [0:1];
    logic signed [3:0] abys_dumper_tmp106;
    logic signed [3:0] abys_dumper_tmp108;
    logic [7:0] abys_dumper_tmp109 [0:1];
    logic signed [4:0] abys_dumper_tmp110;
    logic signed [4:0] abys_dumper_tmp112;
    logic [7:0] abys_dumper_tmp113;
    abys_dumper_tmp50 = ascending_descending;
    abys_dumper_tmp9 = 32'sb10;
    abys_dumper_tmp11 = (abys_dumper_tmp9 - 33'sb10);
    abys_dumper_tmp4 = 32'sb110;
    abys_dumper_tmp6 = (abys_dumper_tmp4 - 33'sb110);
    abys_dumper_tmp50[abys_dumper_tmp11][abys_dumper_tmp6] = value_2_6;
    abys_dumper_tmp22 = 32'sb10;
    abys_dumper_tmp24 = (abys_dumper_tmp22 - 33'sb10);
    abys_dumper_tmp17 = 32'sb111;
    abys_dumper_tmp19 = (abys_dumper_tmp17 - 33'sb110);
    abys_dumper_tmp50[abys_dumper_tmp24][abys_dumper_tmp19] = value_2_7;
    abys_dumper_tmp34 = 32'sb11;
    abys_dumper_tmp36 = (abys_dumper_tmp34 - 33'sb10);
    abys_dumper_tmp29 = 32'sb110;
    abys_dumper_tmp31 = (abys_dumper_tmp29 - 33'sb110);
    abys_dumper_tmp50[abys_dumper_tmp36][abys_dumper_tmp31] = value_3_6;
    abys_dumper_tmp46 = 32'sb11;
    abys_dumper_tmp48 = (abys_dumper_tmp46 - 33'sb10);
    abys_dumper_tmp41 = 32'sb111;
    abys_dumper_tmp43 = (abys_dumper_tmp41 - 33'sb110);
    abys_dumper_tmp50[abys_dumper_tmp48][abys_dumper_tmp43] = value_3_7;
    abys_dumper_tmp97 = row;
    abys_dumper_tmp99 = (abys_dumper_tmp97 - 4'sb10);
    abys_dumper_tmp100 = abys_dumper_tmp50[abys_dumper_tmp99];
    abys_dumper_tmp102 = column;
    abys_dumper_tmp104 = (abys_dumper_tmp102 - 5'sb110);
    abys_dumper_tmp105 = abys_dumper_tmp100[abys_dumper_tmp104];
    abys_dumper_tmp57 = 32'sb10;
    abys_dumper_tmp59 = (abys_dumper_tmp57 - 33'sb10);
    abys_dumper_tmp52 = 32'sb110;
    abys_dumper_tmp54 = (abys_dumper_tmp52 - 33'sb110);
    abys_dumper_tmp69 = 32'sb10;
    abys_dumper_tmp71 = (abys_dumper_tmp69 - 33'sb10);
    abys_dumper_tmp64 = 32'sb111;
    abys_dumper_tmp66 = (abys_dumper_tmp64 - 33'sb110);
    abys_dumper_tmp80 = 32'sb11;
    abys_dumper_tmp82 = (abys_dumper_tmp80 - 33'sb10);
    abys_dumper_tmp75 = 32'sb110;
    abys_dumper_tmp77 = (abys_dumper_tmp75 - 33'sb110);
    abys_dumper_tmp91 = 32'sb11;
    abys_dumper_tmp93 = (abys_dumper_tmp91 - 33'sb10);
    abys_dumper_tmp86 = 32'sb111;
    abys_dumper_tmp88 = (abys_dumper_tmp86 - 33'sb110);
    abys_dumper_tmp95 = descending_ascending;
    abys_dumper_tmp95[abys_dumper_tmp59][abys_dumper_tmp54] = value_2_6;
    abys_dumper_tmp95[abys_dumper_tmp71][abys_dumper_tmp66] = value_2_7;
    abys_dumper_tmp95[abys_dumper_tmp82][abys_dumper_tmp77] = value_3_6;
    abys_dumper_tmp95[abys_dumper_tmp93][abys_dumper_tmp88] = value_3_7;
    abys_dumper_tmp106 = row;
    abys_dumper_tmp108 = (abys_dumper_tmp106 - 4'sb10);
    abys_dumper_tmp109 = abys_dumper_tmp95[abys_dumper_tmp108];
    abys_dumper_tmp110 = column;
    abys_dumper_tmp112 = (abys_dumper_tmp110 - 5'sb110);
    abys_dumper_tmp113 = abys_dumper_tmp109[abys_dumper_tmp112];
    ascending_descending_value = abys_dumper_tmp105;
    descending_ascending[abys_dumper_tmp59][abys_dumper_tmp54] = value_2_6;
    descending_ascending[abys_dumper_tmp71][abys_dumper_tmp66] = value_2_7;
    descending_ascending[abys_dumper_tmp82][abys_dumper_tmp77] = value_3_6;
    descending_ascending[abys_dumper_tmp93][abys_dumper_tmp88] = value_3_7;
    descending_ascending_value = abys_dumper_tmp113;
    ascending_descending[abys_dumper_tmp11][abys_dumper_tmp6] = value_2_6;
    ascending_descending[abys_dumper_tmp24][abys_dumper_tmp19] = value_2_7;
    ascending_descending[abys_dumper_tmp36][abys_dumper_tmp31] = value_3_6;
    ascending_descending[abys_dumper_tmp48][abys_dumper_tmp43] = value_3_7;
  end
endmodule
