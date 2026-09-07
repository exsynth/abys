module top (
  input [7:0] values [0:0],
  output  logic [7:0] result);

  logic [7:0] updated [0:0];


  always @(*)   begin
    logic [7:0] abys_dumper_tmp9 [0:0];
    logic signed [32:0] abys_dumper_tmp6;
    logic [7:0] abys_dumper_tmp11;
    abys_dumper_tmp9 = updated;
    abys_dumper_tmp6 = 32'sb0;
    abys_dumper_tmp9[abys_dumper_tmp6 +: 1'b1] = values;
    abys_dumper_tmp11 = abys_dumper_tmp9[32'sb0];
    result = abys_dumper_tmp11;
    updated[abys_dumper_tmp6 +: 1'b1] = values;
  end
endmodule
