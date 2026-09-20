module top (
  input [3:0] a,
  input [3:0] b,
  input select,
  output  logic y);



  always @(*)   begin
    logic [3:0] abys_dumper_tmp5;
    logic abys_dumper_tmp7;
    logic abys_dumper_tmp8;
    logic abys_dumper_tmp9;
    logic abys_dumper_tmp10;
    logic abys_dumper_tmp11;
    logic abys_dumper_tmp12;
    logic abys_dumper_tmp13;
    logic abys_dumper_tmp14;
    logic abys_dumper_tmp15;
    abys_dumper_tmp5 = (a + b);
    abys_dumper_tmp7 = (abys_dumper_tmp5 == 4'b111);
    abys_dumper_tmp8 = (select & abys_dumper_tmp7);
    abys_dumper_tmp9 = (~abys_dumper_tmp8);
    abys_dumper_tmp10 = (~select);
    abys_dumper_tmp11 = (a == b);
    abys_dumper_tmp12 = (abys_dumper_tmp10 & abys_dumper_tmp11);
    abys_dumper_tmp13 = (~abys_dumper_tmp12);
    abys_dumper_tmp14 = (abys_dumper_tmp9 & abys_dumper_tmp13);
    abys_dumper_tmp15 = (~abys_dumper_tmp14);
    y = abys_dumper_tmp15;
  end
endmodule
