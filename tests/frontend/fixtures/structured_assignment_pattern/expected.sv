module top (
  input sign_i,
  input [2:0] data_i,
  output  logic [7:0] packed_o,
  output  logic [2:0] array_o [2:0]);

  logic [7:0] value;
  logic [2:0] array [2:0];


  always @(*)   begin
    logic [3:0] abys_dumper_tmp3;
    logic [7:0] abys_dumper_tmp5;
    abys_dumper_tmp3 = 1'b1;
    abys_dumper_tmp5 = {sign_i, abys_dumper_tmp3, data_i};
    value = abys_dumper_tmp5;
  end
  always @(*)   begin
    array[2] = 3'b101;
    array[1] = data_i;
    array[0] = 3'b101;
  end
  always @(*)   begin
    packed_o = value;
  end
  always @(*)   begin
    array_o = array;
  end
endmodule
