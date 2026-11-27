module top (
  input [3:0] a,
  output  logic [3:0] y);



  always @(*)   begin
    logic abys_dumper_tmp8;
    logic [3:0] abys_dumper_tmp9;
    logic abys_dumper_tmp15;
    logic [3:0] abys_dumper_tmp16;
    logic abys_dumper_tmp22;
    logic [3:0] abys_dumper_tmp23;
    logic abys_dumper_tmp29;
    logic [3:0] abys_dumper_tmp30;
    abys_dumper_tmp8 = a[2'b11];
    abys_dumper_tmp9 = 4'b0;
    abys_dumper_tmp9[2'b11] = abys_dumper_tmp8;
    abys_dumper_tmp15 = a[2'b10];
    abys_dumper_tmp16 = abys_dumper_tmp9;
    abys_dumper_tmp16[2'b10] = abys_dumper_tmp15;
    abys_dumper_tmp22 = a[1'b1];
    abys_dumper_tmp23 = abys_dumper_tmp16;
    abys_dumper_tmp23[1'b1] = abys_dumper_tmp22;
    abys_dumper_tmp29 = a[1'b0];
    abys_dumper_tmp30 = abys_dumper_tmp23;
    abys_dumper_tmp30[1'b0] = abys_dumper_tmp29;
    y = abys_dumper_tmp30;
  end
endmodule
