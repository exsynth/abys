module top (
  input a,
  input b,
  input c,
  input d,
  output  logic y);



  always @(*)   begin
    logic abys_dumper_tmp4;
    logic abys_dumper_tmp7;
    logic abys_dumper_tmp8;
    abys_dumper_tmp4 = (b & a);
    abys_dumper_tmp7 = (d & c);
    abys_dumper_tmp8 = (abys_dumper_tmp4 & abys_dumper_tmp7);
    y = abys_dumper_tmp8;
  end
endmodule
