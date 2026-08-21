module top (
  input select_i,
  input [3:0] data_i,
  output  logic [3:0] data_o);



  always @(*)   begin
      logic [3:0] abys_dumper_tmp12 [0:1];
      logic [3:0] abys_dumper_tmp14;
      abys_dumper_tmp12[0] = 4'bxxxx;
      abys_dumper_tmp12[1] = 4'bxxxx;
      if (select_i) begin
        abys_dumper_tmp12[0] = 4'bxxxx;
        abys_dumper_tmp12[1] = 4'bxxxx;
        abys_dumper_tmp12[32'sb0] = data_i;
      end else begin
      end
      abys_dumper_tmp14 = abys_dumper_tmp12[32'sb0];
    if (select_i) begin
      data_o = abys_dumper_tmp14;
    end else begin
      data_o = 4'b0;
    end
  end
endmodule
