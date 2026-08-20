module top (
  input [7:0] values [0:0],
  output  logic [7:0] result);

  logic [7:0] updated [0:0];


  always @(*)   begin
    logic [7:0] abys_dumper_tmp7 [0:0];
    logic [7:0] abys_dumper_tmp9;
    abys_dumper_tmp7 = updated;
    abys_dumper_tmp7[32'sb0 +: 1'b1] = values;
    abys_dumper_tmp9 = abys_dumper_tmp7[32'sb0];
    result = abys_dumper_tmp9;
    updated[32'sb0 +: 1'b1] = values;
  end
endmodule
