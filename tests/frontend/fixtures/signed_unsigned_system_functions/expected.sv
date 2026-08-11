module top (
  input signed [3:0] signed_i,
  input [3:0] unsigned_i,
  output  logic signed [3:0] signed_o,
  output  logic [3:0] unsigned_o);



  always @(*)   begin
    logic signed [3:0] abys_dumper_tmp3;
    abys_dumper_tmp3 = unsigned_i;
    signed_o = abys_dumper_tmp3;
  end
  always @(*)   begin
    logic [3:0] abys_dumper_tmp3;
    abys_dumper_tmp3 = signed_i;
    unsigned_o = abys_dumper_tmp3;
  end
endmodule
