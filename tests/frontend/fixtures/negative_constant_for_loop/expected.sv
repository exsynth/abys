module top (
  input [3:0] a,
  output  logic [3:0] y);



  always @(*)   begin
    logic abys_dumper_tmp12;
    logic [3:0] abys_dumper_tmp15;
    logic abys_dumper_tmp23;
    logic [3:0] abys_dumper_tmp26;
    logic abys_dumper_tmp35;
    logic [3:0] abys_dumper_tmp39;
    logic abys_dumper_tmp48;
    logic [3:0] abys_dumper_tmp52;
    abys_dumper_tmp12 = a[1'b0];
    abys_dumper_tmp15 = 4'b0;
    abys_dumper_tmp15[1'b0] = abys_dumper_tmp12;
    abys_dumper_tmp23 = a[1'b1];
    abys_dumper_tmp26 = abys_dumper_tmp15;
    abys_dumper_tmp26[1'b1] = abys_dumper_tmp23;
    abys_dumper_tmp35 = a[2'b10];
    abys_dumper_tmp39 = abys_dumper_tmp26;
    abys_dumper_tmp39[2'b10] = abys_dumper_tmp35;
    abys_dumper_tmp48 = a[2'b11];
    abys_dumper_tmp52 = abys_dumper_tmp39;
    abys_dumper_tmp52[2'b11] = abys_dumper_tmp48;
    y = abys_dumper_tmp52;
  end
endmodule
