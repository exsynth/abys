module top (
  input select_i,
  input [3:0] data_i,
  output  logic [3:0] data_o);



  always @(*)   begin
    logic [3:0] abys_dumper_tmp15 [1:0];
      logic signed [32:0] abys_dumper_tmp9;
      logic signed [32:0] abys_dumper_tmp11;
    logic signed [32:0] abys_dumper_tmp17;
    logic signed [32:0] abys_dumper_tmp19;
    logic [3:0] abys_dumper_tmp20;
    abys_dumper_tmp15[1] = 4'bxxxx;
    abys_dumper_tmp15[0] = 4'bxxxx;
      abys_dumper_tmp9 = 32'sb0;
      abys_dumper_tmp11 = (33'sb1 - abys_dumper_tmp9);
    if (select_i) begin
      abys_dumper_tmp15[1] = 4'bxxxx;
      abys_dumper_tmp15[0] = 4'bxxxx;
      abys_dumper_tmp15[abys_dumper_tmp11] = data_i;
    end else begin
    end
    abys_dumper_tmp17 = 32'sb0;
    abys_dumper_tmp19 = (33'sb1 - abys_dumper_tmp17);
    abys_dumper_tmp20 = abys_dumper_tmp15[abys_dumper_tmp19];
    data_o = abys_dumper_tmp20;
  end
endmodule
