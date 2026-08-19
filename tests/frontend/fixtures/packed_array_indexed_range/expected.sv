module top (
  input [23:0] data_i,
  input [23:0] ascending_i,
  input [15:0] update_i,
  input base_i,
  output  logic [15:0] up_o,
  output  logic [15:0] down_o,
  output  logic [15:0] ascending_up_o,
  output  logic [15:0] ascending_down_o,
  output  logic [23:0] updated_up_o,
  output  logic [23:0] updated_down_o);



  always @(*)   begin
    logic [15:0] abys_dumper_tmp6;
    logic [3:0] abys_dumper_tmp5;
    abys_dumper_tmp5 = (base_i * 4'b1000);
    abys_dumper_tmp6 = data_i[abys_dumper_tmp5 +: 16];
    up_o = abys_dumper_tmp6;
  end
  always @(*)   begin
    logic [15:0] abys_dumper_tmp15;
    logic [31:0] abys_dumper_tmp4;
    logic [31:0] abys_dumper_tmp6;
    logic [31:0] abys_dumper_tmp7;
    logic signed [34:0] abys_dumper_tmp8;
    logic signed [34:0] abys_dumper_tmp10;
    logic signed [34:0] abys_dumper_tmp12;
    logic [34:0] abys_dumper_tmp14;
    abys_dumper_tmp4 = base_i;
    abys_dumper_tmp6 = 32'sb1;
    abys_dumper_tmp7 = (abys_dumper_tmp4 + abys_dumper_tmp6);
    abys_dumper_tmp8 = abys_dumper_tmp7;
    abys_dumper_tmp10 = (abys_dumper_tmp8 + -35'sb1);
    abys_dumper_tmp12 = (abys_dumper_tmp10 - 35'sb0);
    abys_dumper_tmp14 = (abys_dumper_tmp12 * 4'b1000);
    abys_dumper_tmp15 = data_i[abys_dumper_tmp14 +: 16];
    down_o = abys_dumper_tmp15;
  end
  always @(*)   begin
    logic [15:0] abys_dumper_tmp11;
    logic signed [4:0] abys_dumper_tmp4;
    logic signed [4:0] abys_dumper_tmp6;
    logic signed [4:0] abys_dumper_tmp8;
    logic [4:0] abys_dumper_tmp10;
    abys_dumper_tmp4 = base_i;
    abys_dumper_tmp6 = (abys_dumper_tmp4 + 5'sb1);
    abys_dumper_tmp8 = (5'sb10 - abys_dumper_tmp6);
    abys_dumper_tmp10 = (abys_dumper_tmp8 * 4'b1000);
    abys_dumper_tmp11 = ascending_i[abys_dumper_tmp10 +: 16];
    ascending_up_o = abys_dumper_tmp11;
  end
  always @(*)   begin
    logic [15:0] abys_dumper_tmp13;
    logic [31:0] abys_dumper_tmp4;
    logic [31:0] abys_dumper_tmp6;
    logic [31:0] abys_dumper_tmp7;
    logic signed [33:0] abys_dumper_tmp8;
    logic signed [33:0] abys_dumper_tmp10;
    logic [33:0] abys_dumper_tmp12;
    abys_dumper_tmp4 = base_i;
    abys_dumper_tmp6 = 32'sb1;
    abys_dumper_tmp7 = (abys_dumper_tmp4 + abys_dumper_tmp6);
    abys_dumper_tmp8 = abys_dumper_tmp7;
    abys_dumper_tmp10 = (34'sb10 - abys_dumper_tmp8);
    abys_dumper_tmp12 = (abys_dumper_tmp10 * 4'b1000);
    abys_dumper_tmp13 = ascending_i[abys_dumper_tmp12 +: 16];
    ascending_down_o = abys_dumper_tmp13;
  end
  always @(*)   begin
    logic [31:0] abys_dumper_tmp10;
    logic [31:0] abys_dumper_tmp12;
    logic [31:0] abys_dumper_tmp13;
    logic signed [34:0] abys_dumper_tmp14;
    logic signed [34:0] abys_dumper_tmp16;
    logic signed [34:0] abys_dumper_tmp18;
    logic [34:0] abys_dumper_tmp20;
    logic [34:0] abys_dumper_tmp21;
    logic [23:0] abys_dumper_tmp23;
    logic [3:0] abys_dumper_tmp6;
    logic [3:0] abys_dumper_tmp7;
    logic [23:0] abys_dumper_tmp9;
    abys_dumper_tmp10 = base_i;
    abys_dumper_tmp12 = 32'sb1;
    abys_dumper_tmp13 = (abys_dumper_tmp10 + abys_dumper_tmp12);
    abys_dumper_tmp14 = abys_dumper_tmp13;
    abys_dumper_tmp16 = (abys_dumper_tmp14 + -35'sb1);
    abys_dumper_tmp18 = (abys_dumper_tmp16 - 35'sb0);
    abys_dumper_tmp20 = (abys_dumper_tmp18 * 4'b1000);
    abys_dumper_tmp21 = (1'b0 + abys_dumper_tmp20);
    abys_dumper_tmp23 = data_i;
    abys_dumper_tmp23[abys_dumper_tmp21 +: 5'b10000] = update_i;
    abys_dumper_tmp6 = (base_i * 4'b1000);
    abys_dumper_tmp7 = (1'b0 + abys_dumper_tmp6);
    abys_dumper_tmp9 = data_i;
    abys_dumper_tmp9[abys_dumper_tmp7 +: 5'b10000] = update_i;
    updated_down_o = abys_dumper_tmp23;
    updated_up_o = abys_dumper_tmp9;
  end
endmodule
