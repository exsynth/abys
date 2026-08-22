module top (
  input [3:0] upper,
  input [3:0] lower,
  output  logic [7:0] y);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp6;
    logic [7:0] abys_dumper_tmp9;
    abys_dumper_tmp6 = 8'bxxxxxxxx;
    abys_dumper_tmp6[3'b100 +: 3'b100] = upper;
    abys_dumper_tmp9 = abys_dumper_tmp6;
    abys_dumper_tmp9[1'b0 +: 3'b100] = lower;
    y = abys_dumper_tmp9;
  end
endmodule
