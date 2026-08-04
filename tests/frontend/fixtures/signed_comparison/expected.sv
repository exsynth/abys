module top (
  input signed [7:0] a,
  input signed [7:0] b,
  output  logic y);



  always @(*)   begin
    logic abys_dumper_tmp4;
    abys_dumper_tmp4 = (a < b);
    y = abys_dumper_tmp4;
  end
endmodule
