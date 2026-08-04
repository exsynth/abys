module top (
  input select,
  input [7:0] a,
  input [7:0] b,
  output  logic [7:0] y);

  logic [7:0] memory [1:0];


  always @(*)   begin
    logic [7:0] abys_dumper_tmp31 [1:0];
    logic signed [32:0] abys_dumper_tmp4;
    logic signed [32:0] abys_dumper_tmp6;
    logic signed [32:0] abys_dumper_tmp12;
    logic signed [32:0] abys_dumper_tmp14;
      logic signed [32:0] abys_dumper_tmp19;
      logic signed [32:0] abys_dumper_tmp21;
      logic signed [32:0] abys_dumper_tmp25;
      logic signed [32:0] abys_dumper_tmp27;
    logic signed [32:0] abys_dumper_tmp33;
    logic signed [32:0] abys_dumper_tmp35;
    logic [7:0] abys_dumper_tmp36;
    logic signed [32:0] abys_dumper_tmp38;
    logic signed [32:0] abys_dumper_tmp40;
    logic [7:0] abys_dumper_tmp41;
    logic [7:0] abys_dumper_tmp42;
    abys_dumper_tmp31 = memory;
    abys_dumper_tmp4 = 32'sb0;
    abys_dumper_tmp6 = (33'sb1 - abys_dumper_tmp4);
    abys_dumper_tmp31[abys_dumper_tmp6] = a;
    abys_dumper_tmp12 = 32'sb1;
    abys_dumper_tmp14 = (33'sb1 - abys_dumper_tmp12);
    abys_dumper_tmp31[abys_dumper_tmp14] = b;
      abys_dumper_tmp19 = 32'sb0;
      abys_dumper_tmp21 = (33'sb1 - abys_dumper_tmp19);
      abys_dumper_tmp25 = 32'sb1;
      abys_dumper_tmp27 = (33'sb1 - abys_dumper_tmp25);
    if (select) begin
      abys_dumper_tmp31[abys_dumper_tmp6] = a;
      abys_dumper_tmp31[abys_dumper_tmp14] = b;
      abys_dumper_tmp31[abys_dumper_tmp21] = b;
    end else begin
      abys_dumper_tmp31[abys_dumper_tmp6] = a;
      abys_dumper_tmp31[abys_dumper_tmp14] = b;
      abys_dumper_tmp31[abys_dumper_tmp27] = a;
    end
    abys_dumper_tmp33 = 32'sb0;
    abys_dumper_tmp35 = (33'sb1 - abys_dumper_tmp33);
    abys_dumper_tmp36 = abys_dumper_tmp31[abys_dumper_tmp35];
    abys_dumper_tmp38 = 32'sb1;
    abys_dumper_tmp40 = (33'sb1 - abys_dumper_tmp38);
    abys_dumper_tmp41 = abys_dumper_tmp31[abys_dumper_tmp40];
    abys_dumper_tmp42 = (abys_dumper_tmp36 ^ abys_dumper_tmp41);
    y = abys_dumper_tmp42;
    memory[abys_dumper_tmp6] = a;
    memory[abys_dumper_tmp14] = b;
    if (select) begin
      memory[abys_dumper_tmp6] = a;
      memory[abys_dumper_tmp14] = b;
      memory[abys_dumper_tmp21] = b;
    end else begin
      memory[abys_dumper_tmp6] = a;
      memory[abys_dumper_tmp14] = b;
      memory[abys_dumper_tmp27] = a;
    end
  end
endmodule
