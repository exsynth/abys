module top (
  input [7:0] a,
  input enable,
  output  logic [7:0] y);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp4;
    logic [7:0] abys_dumper_tmp5;
    logic [7:0] abys_dumper_tmp6;
    abys_dumper_tmp4 = (~a);
    if (enable) begin
      abys_dumper_tmp5 = abys_dumper_tmp4;
    end else begin
      abys_dumper_tmp5 = a;
    end
    abys_dumper_tmp6 = abys_dumper_tmp5;
    y = abys_dumper_tmp6;
  end
endmodule
