module top (
  input [7:0] a,
  output  logic [7:0] y);


function automatic [7:0] invert (
  input [7:0] value
);
  begin
    logic [7:0] abys_dumper_tmp4;
    abys_dumper_tmp4 = (~value);
    invert = abys_dumper_tmp4;
  end
endfunction

function automatic [7:0] invert_twice (
  input [7:0] value
);
  begin
    logic [7:0] abys_dumper_tmp5;
    logic [7:0] abys_dumper_tmp4;
    abys_dumper_tmp4 = invert(value);
    abys_dumper_tmp5 = invert(abys_dumper_tmp4);
    invert_twice = abys_dumper_tmp5;
  end
endfunction


  always @(*)   begin
    logic [7:0] abys_dumper_tmp3;
    abys_dumper_tmp3 = invert_twice(a);
    y = abys_dumper_tmp3;
  end
endmodule
