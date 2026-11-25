module top (
  input [3:0] a,
  output  logic [3:0] y);



  always @(*)   begin
    logic abys_dumper_tmp12;
    logic [3:0] abys_dumper_tmp15;
    logic abys_dumper_tmp23;
    logic [3:0] abys_dumper_tmp26;
    logic abys_dumper_tmp34;
    logic [3:0] abys_dumper_tmp37;
    logic abys_dumper_tmp45;
    logic [3:0] abys_dumper_tmp48;
    abys_dumper_tmp12 = a[1'b0];
    abys_dumper_tmp15 = 4'b0;
    abys_dumper_tmp15[1'b0] = abys_dumper_tmp12;
    abys_dumper_tmp23 = a[1'b1];
    abys_dumper_tmp26 = abys_dumper_tmp15;
    abys_dumper_tmp26[1'b1] = abys_dumper_tmp23;
    abys_dumper_tmp34 = a[2'b10];
    abys_dumper_tmp37 = abys_dumper_tmp26;
    abys_dumper_tmp37[2'b10] = abys_dumper_tmp34;
    abys_dumper_tmp45 = a[2'b11];
    abys_dumper_tmp48 = abys_dumper_tmp37;
    abys_dumper_tmp48[2'b11] = abys_dumper_tmp45;
    y = abys_dumper_tmp48;
  end
endmodule
