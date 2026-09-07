module top (
  input clk,
  input comb_data,
  input edge_data,
  output  logic [1:0] q);

  logic [1:0] abys_builder_tmp0;


  always @(*) begin
    q = abys_builder_tmp0;
    begin
      q[1'b1] = comb_data;
    end
  end
  always @(posedge clk) begin
    begin
      logic [1:0] abys_dumper_tmp6;
      abys_dumper_tmp6 = q;
      abys_dumper_tmp6[1'b0] = edge_data;
      abys_builder_tmp0 <= abys_dumper_tmp6;
    end
  end
endmodule
