module top (
  input [3:0] a,
  input [3:0] b,
  output  logic [3:0] y);



  always @(*)   begin
    logic abys_dumper_tmp4;
    logic abys_dumper_tmp6;
    logic abys_dumper_tmp7;
    logic abys_dumper_tmp8;
    logic [3:0] abys_dumper_tmp9;
    logic [3:0] abys_dumper_tmp11;
    abys_dumper_tmp4 = a[2'b11];
    abys_dumper_tmp6 = a[2'b10];
    abys_dumper_tmp7 = a[1'b1];
    abys_dumper_tmp8 = a[1'b0];
    abys_dumper_tmp9 = {abys_dumper_tmp4, abys_dumper_tmp6, abys_dumper_tmp7, abys_dumper_tmp8};
    abys_dumper_tmp11 = (abys_dumper_tmp9 + b);
    y = abys_dumper_tmp11;
  end
endmodule
