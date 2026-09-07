module top (
  input select,
  output  logic result);

  logic generated_value;


  always @(*)   begin
    logic abys_dumper_tmp4;
    abys_dumper_tmp4 = (select & generated_value);
    result = abys_dumper_tmp4;
  end
  always @(*)   begin
    generated_value = 1'b0;
  end
endmodule
