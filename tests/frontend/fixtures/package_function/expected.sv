function automatic [7:0] increment (
  input [7:0] value
);
  begin
    logic [7:0] abys_dumper_tmp5;
    abys_dumper_tmp5 = (value + 8'b1);
    increment = abys_dumper_tmp5;
  end
endfunction

module top (
  input [7:0] value_i,
  output  logic [7:0] value_o);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp3;
    abys_dumper_tmp3 = increment(value_i);
    value_o = abys_dumper_tmp3;
  end
endmodule
