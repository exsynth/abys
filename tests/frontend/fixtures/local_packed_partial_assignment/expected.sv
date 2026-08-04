module top (
  input [3:0] upper,
  input [3:0] lower,
  output  logic [7:0] y);



  always @(*)   begin
    logic [2:0] abys_dumper_tmp4;
    logic [7:0] abys_dumper_tmp7;
    logic abys_dumper_tmp9;
    logic [7:0] abys_dumper_tmp11;
    abys_dumper_tmp4 = (1'b0 + 3'b100);
    abys_dumper_tmp7[abys_dumper_tmp4 +: 3'b100] = upper;
    abys_dumper_tmp9 = (1'b0 + 1'b0);
    abys_dumper_tmp11 = abys_dumper_tmp7;
    abys_dumper_tmp11[abys_dumper_tmp9 +: 3'b100] = lower;
    y = abys_dumper_tmp11;
  end
endmodule
