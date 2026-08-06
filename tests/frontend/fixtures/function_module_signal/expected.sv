module top (
  input value,
  input enable,
  output  logic y);


function automatic filter (
  input argument
);
  begin
    logic abys_dumper_tmp5;
    abys_dumper_tmp5 = (argument & enable);
    filter = abys_dumper_tmp5;
  end
endfunction


  always @(*)   begin
    logic abys_dumper_tmp3;
    abys_dumper_tmp3 = filter(value);
    y = abys_dumper_tmp3;
  end
endmodule
