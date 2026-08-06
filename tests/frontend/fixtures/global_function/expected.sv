function automatic [7:0] invert (
  input [7:0] value
);
  begin
    logic [7:0] abys_dumper_tmp4;
    abys_dumper_tmp4 = (~value);
    invert = abys_dumper_tmp4;
  end
endfunction

module top (
  input [7:0] a,
  output  logic [7:0] y);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp3;
    abys_dumper_tmp3 = invert(a);
    y = abys_dumper_tmp3;
  end
endmodule
