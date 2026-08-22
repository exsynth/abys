module top (
  input [7:0] d_i,
  output  logic [7:0] d_o);


function automatic [7:0] reverse (
  input [7:0] value
);
  begin
    logic abys_dumper_tmp6;
    logic abys_dumper_tmp8;
    logic abys_dumper_tmp11;
    logic abys_dumper_tmp14;
    logic [3:0] abys_dumper_tmp15;
    logic [7:0] abys_dumper_tmp18;
    logic abys_dumper_tmp21;
    logic abys_dumper_tmp24;
    logic abys_dumper_tmp27;
    logic abys_dumper_tmp30;
    logic [3:0] abys_dumper_tmp31;
    logic [7:0] abys_dumper_tmp33;
    abys_dumper_tmp6 = value[1'b0];
    abys_dumper_tmp8 = value[1'b1];
    abys_dumper_tmp11 = value[2'b10];
    abys_dumper_tmp14 = value[2'b11];
    abys_dumper_tmp15 = {abys_dumper_tmp6, abys_dumper_tmp8, abys_dumper_tmp11, abys_dumper_tmp14};
    abys_dumper_tmp18 = 8'bxxxxxxxx;
    abys_dumper_tmp18[3'b100 +: 3'b100] = abys_dumper_tmp15;
    abys_dumper_tmp21 = value[3'b100];
    abys_dumper_tmp24 = value[3'b101];
    abys_dumper_tmp27 = value[3'b110];
    abys_dumper_tmp30 = value[3'b111];
    abys_dumper_tmp31 = {abys_dumper_tmp21, abys_dumper_tmp24, abys_dumper_tmp27, abys_dumper_tmp30};
    abys_dumper_tmp33 = abys_dumper_tmp18;
    abys_dumper_tmp33[1'b0 +: 3'b100] = abys_dumper_tmp31;
    reverse = abys_dumper_tmp33;
  end
endfunction


  always @(*)   begin
    logic [7:0] abys_dumper_tmp3;
    abys_dumper_tmp3 = reverse(d_i);
    d_o = abys_dumper_tmp3;
  end
endmodule
