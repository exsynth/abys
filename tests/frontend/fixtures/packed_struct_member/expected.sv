module top (
  input [3:0] value_i,
  output  logic [3:0] value_o,
  output  logic flag_o,
  output  logic [2:0] data_o);



  always @(*)   begin
    logic abys_dumper_tmp4;
    abys_dumper_tmp4 = value_i[2'b11];
    flag_o = abys_dumper_tmp4;
  end
  always @(*)   begin
    logic [2:0] abys_dumper_tmp3;
    abys_dumper_tmp3 = value_i[1'b0 +: 3];
    data_o = abys_dumper_tmp3;
  end
  always @(*) begin
    begin
      logic [3:0] abys_dumper_tmp4;
      abys_dumper_tmp4 = (1'b0 + 4'd3);
      value_o[abys_dumper_tmp4] = flag_o;
    end
    begin
      logic [3:0] abys_dumper_tmp4;
      abys_dumper_tmp4 = (1'b0 + 4'd0);
      value_o[abys_dumper_tmp4 +: 2'b11] = data_o;
    end
  end
endmodule
