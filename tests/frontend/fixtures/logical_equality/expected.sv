module top (
  input valid,
  input [7:0] a,
  input [7:0] b,
  output  logic y);



  always @(*)   begin
    logic abys_dumper_tmp5;
    logic abys_dumper_tmp6;
    abys_dumper_tmp5 = (a == b);
    abys_dumper_tmp6 = (valid & abys_dumper_tmp5);
    y = abys_dumper_tmp6;
  end
endmodule
