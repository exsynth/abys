module top (
  output  logic [3:0] y);

  logic [3:0] values [1:0];


  always @(*)   begin
    values[1] = 4'b1010;
    values[0] = 4'b101;
  end
  always @(*)   begin
    logic signed [32:0] abys_dumper_tmp4;
    logic signed [32:0] abys_dumper_tmp6;
    logic [3:0] abys_dumper_tmp7;
    abys_dumper_tmp4 = 32'sb0;
    abys_dumper_tmp6 = (33'sb1 - abys_dumper_tmp4);
    abys_dumper_tmp7 = values[abys_dumper_tmp6];
    y = abys_dumper_tmp7;
  end
endmodule
