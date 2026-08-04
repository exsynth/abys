module top (
  input [7:0] a,
  output  logic y);



  always @(*)   begin
    logic abys_dumper_tmp3;
    abys_dumper_tmp3 = (^a);
    y = abys_dumper_tmp3;
  end
endmodule
