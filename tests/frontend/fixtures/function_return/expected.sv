module top (
  input [7:0] d_i,
  output  logic [7:0] d_o);


function automatic [7:0] reverse (
  input [7:0] value
);
  begin
    logic abys_dumper_tmp5;
    logic abys_dumper_tmp7;
    logic abys_dumper_tmp9;
    logic abys_dumper_tmp11;
    logic abys_dumper_tmp13;
    logic abys_dumper_tmp15;
    logic abys_dumper_tmp17;
    logic abys_dumper_tmp19;
    logic [7:0] abys_dumper_tmp20;
    abys_dumper_tmp5 = value[32'sb0];
    abys_dumper_tmp7 = value[32'sb1];
    abys_dumper_tmp9 = value[32'sb10];
    abys_dumper_tmp11 = value[32'sb11];
    abys_dumper_tmp13 = value[32'sb100];
    abys_dumper_tmp15 = value[32'sb101];
    abys_dumper_tmp17 = value[32'sb110];
    abys_dumper_tmp19 = value[32'sb111];
    abys_dumper_tmp20 = {abys_dumper_tmp5, abys_dumper_tmp7, abys_dumper_tmp9, abys_dumper_tmp11, abys_dumper_tmp13, abys_dumper_tmp15, abys_dumper_tmp17, abys_dumper_tmp19};
    reverse = abys_dumper_tmp20;
  end
endfunction


  always @(*)   begin
    logic [7:0] abys_dumper_tmp3;
    abys_dumper_tmp3 = reverse(d_i);
    d_o = abys_dumper_tmp3;
  end
endmodule
