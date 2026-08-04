module top (
  input row,
  input column,
  input [7:0] value,
  output  logic [7:0] y);

  logic [7:0] memory [1:0] [1:0];


  always @(*)   begin
    logic [7:0] abys_dumper_tmp62 [1:0] [1:0];
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
    logic signed [2:0] abys_dumper_tmp58;
    logic signed [2:0] abys_dumper_tmp60;
    logic signed [2:0] abys_dumper_tmp53;
    logic signed [2:0] abys_dumper_tmp55;
    logic signed [32:0] abys_dumper_tmp64;
    logic signed [32:0] abys_dumper_tmp66;
    logic [7:0] abys_dumper_tmp67 [1:0];
    logic signed [32:0] abys_dumper_tmp69;
    logic signed [32:0] abys_dumper_tmp71;
    logic [7:0] abys_dumper_tmp72;
    abys_dumper_tmp62 = memory;
    abys_dumper_tmp9 = 32'sb0;
    abys_dumper_tmp11 = (33'sb1 - abys_dumper_tmp9);
    abys_dumper_tmp4 = 32'sb0;
    abys_dumper_tmp6 = (33'sb1 - abys_dumper_tmp4);
    abys_dumper_tmp62[abys_dumper_tmp11][abys_dumper_tmp6] = 8'b10001;
    abys_dumper_tmp22 = 32'sb0;
    abys_dumper_tmp24 = (33'sb1 - abys_dumper_tmp22);
    abys_dumper_tmp17 = 32'sb1;
    abys_dumper_tmp19 = (33'sb1 - abys_dumper_tmp17);
    abys_dumper_tmp62[abys_dumper_tmp24][abys_dumper_tmp19] = 8'b100010;
    abys_dumper_tmp34 = 32'sb1;
    abys_dumper_tmp36 = (33'sb1 - abys_dumper_tmp34);
    abys_dumper_tmp29 = 32'sb0;
    abys_dumper_tmp31 = (33'sb1 - abys_dumper_tmp29);
    abys_dumper_tmp62[abys_dumper_tmp36][abys_dumper_tmp31] = 8'b110011;
    abys_dumper_tmp46 = 32'sb1;
    abys_dumper_tmp48 = (33'sb1 - abys_dumper_tmp46);
    abys_dumper_tmp41 = 32'sb1;
    abys_dumper_tmp43 = (33'sb1 - abys_dumper_tmp41);
    abys_dumper_tmp62[abys_dumper_tmp48][abys_dumper_tmp43] = 8'b1000100;
    abys_dumper_tmp58 = row;
    abys_dumper_tmp60 = (3'sb1 - abys_dumper_tmp58);
    abys_dumper_tmp53 = column;
    abys_dumper_tmp55 = (3'sb1 - abys_dumper_tmp53);
    abys_dumper_tmp62[abys_dumper_tmp60][abys_dumper_tmp55] = value;
    abys_dumper_tmp64 = 32'sb1;
    abys_dumper_tmp66 = (33'sb1 - abys_dumper_tmp64);
    abys_dumper_tmp67 = abys_dumper_tmp62[abys_dumper_tmp66];
    abys_dumper_tmp69 = 32'sb0;
    abys_dumper_tmp71 = (33'sb1 - abys_dumper_tmp69);
    abys_dumper_tmp72 = abys_dumper_tmp67[abys_dumper_tmp71];
    y = abys_dumper_tmp72;
    memory[abys_dumper_tmp11][abys_dumper_tmp6] = 8'b10001;
    memory[abys_dumper_tmp24][abys_dumper_tmp19] = 8'b100010;
    memory[abys_dumper_tmp36][abys_dumper_tmp31] = 8'b110011;
    memory[abys_dumper_tmp48][abys_dumper_tmp43] = 8'b1000100;
    memory[abys_dumper_tmp60][abys_dumper_tmp55] = value;
  end
endmodule
