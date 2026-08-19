module top (
  input [1:0] clocks,
  input [1:0] resets,
  input [1:0] d,
  output  logic [1:0] q);

  logic abys_builder_tmp0;
  logic abys_builder_tmp1;
  logic abys_builder_tmp2;
  logic abys_builder_tmp3;


  always @(*)   begin
    logic abys_dumper_tmp4;
    abys_dumper_tmp4 = clocks[32'sb1];
    abys_builder_tmp0 = abys_dumper_tmp4;
  end
  always @(*)   begin
    logic abys_dumper_tmp7;
    logic signed [32:0] abys_dumper_tmp4;
    logic signed [32:0] abys_dumper_tmp6;
    abys_dumper_tmp4 = 32'sb10;
    abys_dumper_tmp6 = (33'sb11 - abys_dumper_tmp4);
    abys_dumper_tmp7 = resets[abys_dumper_tmp6];
    abys_builder_tmp1 = abys_dumper_tmp7;
  end
  always @(*)   begin
    logic abys_dumper_tmp4;
    abys_dumper_tmp4 = clocks[32'sb0];
    abys_builder_tmp2 = abys_dumper_tmp4;
  end
  always @(*)   begin
    logic abys_dumper_tmp7;
    logic signed [32:0] abys_dumper_tmp4;
    logic signed [32:0] abys_dumper_tmp6;
    abys_dumper_tmp4 = 32'sb11;
    abys_dumper_tmp6 = (33'sb11 - abys_dumper_tmp4);
    abys_dumper_tmp7 = resets[abys_dumper_tmp6];
    abys_builder_tmp3 = abys_dumper_tmp7;
  end
  always @(posedge abys_builder_tmp0 or negedge abys_builder_tmp1) begin
    if (!abys_builder_tmp1) begin
      begin
        logic [31:0] abys_dumper_tmp6;
        abys_dumper_tmp6 = (1'b0 + 32'sb0);
        q[abys_dumper_tmp6] <= 1'b0;
      end
    end else begin
      begin
        logic [31:0] abys_dumper_tmp13;
        logic abys_dumper_tmp11;
        abys_dumper_tmp13 = (1'b0 + 32'sb0);
        abys_dumper_tmp11 = d[32'sb0];
        q[abys_dumper_tmp13] <= abys_dumper_tmp11;
      end
    end
  end
  always @(posedge abys_builder_tmp2 or negedge abys_builder_tmp3) begin
    if (!abys_builder_tmp3) begin
      begin
        logic [31:0] abys_dumper_tmp6;
        abys_dumper_tmp6 = (1'b0 + 32'sb1);
        q[abys_dumper_tmp6] <= 1'b0;
      end
    end else begin
      begin
        logic [31:0] abys_dumper_tmp13;
        logic abys_dumper_tmp11;
        abys_dumper_tmp13 = (1'b0 + 32'sb1);
        abys_dumper_tmp11 = d[32'sb1];
        q[abys_dumper_tmp13] <= abys_dumper_tmp11;
      end
    end
  end
endmodule
