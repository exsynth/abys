module top (
  output  logic [3:0] y);

  logic [3:0] values [0:1];


  always @(*)   begin
    values[0] = 4'b1010;
    values[1] = 4'b101;
  end
  always @(*)   begin
    logic [3:0] abys_dumper_tmp4;
    abys_dumper_tmp4 = values[32'sb0];
    y = abys_dumper_tmp4;
  end
endmodule
