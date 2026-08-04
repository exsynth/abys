module top (
  input [15:0] a,
  input [3:0] index,
  output  logic y);



  always @(*)   begin
    logic abys_dumper_tmp4;
    abys_dumper_tmp4 = a[index];
    y = abys_dumper_tmp4;
  end
endmodule
