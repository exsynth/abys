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
    logic [7:0] abys_dumper_tmp14;
    logic abys_dumper_tmp16;
    logic abys_dumper_tmp18;
    logic abys_dumper_tmp20;
    logic abys_dumper_tmp22;
    logic [3:0] abys_dumper_tmp23;
    logic [7:0] abys_dumper_tmp24;
    abys_dumper_tmp6 = value[1'b0];
    abys_dumper_tmp8 = value[1'b1];
    abys_dumper_tmp10 = value[2'b10];
    abys_dumper_tmp12 = value[2'b11];
    abys_dumper_tmp13 = {abys_dumper_tmp6, abys_dumper_tmp8, abys_dumper_tmp10, abys_dumper_tmp12};
    abys_dumper_tmp14 = 8'bxxxxxxxx;
    abys_dumper_tmp14[3'b100 +: 3'b100] = abys_dumper_tmp13;
    abys_dumper_tmp16 = value[3'b100];
    abys_dumper_tmp18 = value[3'b101];
    abys_dumper_tmp20 = value[3'b110];
    abys_dumper_tmp22 = value[3'b111];
    abys_dumper_tmp23 = {abys_dumper_tmp16, abys_dumper_tmp18, abys_dumper_tmp20, abys_dumper_tmp22};
    abys_dumper_tmp24 = abys_dumper_tmp14;
    abys_dumper_tmp24[1'b0 +: 3'b100] = abys_dumper_tmp23;
    reverse = abys_dumper_tmp24;
  end
endfunction


  always @(*)   begin
    logic [7:0] abys_dumper_tmp3;
    abys_dumper_tmp3 = reverse(d_i);
    d_o = abys_dumper_tmp3;
  end
endmodule
