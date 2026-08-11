module top (
  input [7:0] d_i,
  output  logic [7:0] d_o);


function automatic [7:0] reverse (
  input [7:0] value
);
  begin
    logic abys_dumper_tmp6;
    logic abys_dumper_tmp8;
    logic abys_dumper_tmp10;
    logic abys_dumper_tmp12;
    logic [3:0] abys_dumper_tmp13;
    logic [2:0] abys_dumper_tmp15;
    logic [7:0] abys_dumper_tmp17;
    logic abys_dumper_tmp19;
    logic abys_dumper_tmp21;
    logic abys_dumper_tmp23;
    logic abys_dumper_tmp25;
    logic [3:0] abys_dumper_tmp26;
    logic abys_dumper_tmp27;
    logic [7:0] abys_dumper_tmp29;
    abys_dumper_tmp6 = value[32'sb0];
    abys_dumper_tmp8 = value[32'sb1];
    abys_dumper_tmp10 = value[32'sb10];
    abys_dumper_tmp12 = value[32'sb11];
    abys_dumper_tmp13 = {abys_dumper_tmp6, abys_dumper_tmp8, abys_dumper_tmp10, abys_dumper_tmp12};
    abys_dumper_tmp15 = (1'b0 + 3'b100);
    abys_dumper_tmp17 = 8'bxxxxxxxx;
    abys_dumper_tmp17[abys_dumper_tmp15 +: 3'b100] = abys_dumper_tmp13;
    abys_dumper_tmp19 = value[32'sb100];
    abys_dumper_tmp21 = value[32'sb101];
    abys_dumper_tmp23 = value[32'sb110];
    abys_dumper_tmp25 = value[32'sb111];
    abys_dumper_tmp26 = {abys_dumper_tmp19, abys_dumper_tmp21, abys_dumper_tmp23, abys_dumper_tmp25};
    abys_dumper_tmp27 = (1'b0 + 1'b0);
    abys_dumper_tmp29 = abys_dumper_tmp17;
    abys_dumper_tmp29[abys_dumper_tmp27 +: 3'b100] = abys_dumper_tmp26;
    reverse = abys_dumper_tmp29;
  end
endfunction


  always @(*)   begin
    logic [7:0] abys_dumper_tmp3;
    abys_dumper_tmp3 = reverse(d_i);
    d_o = abys_dumper_tmp3;
  end
endmodule
