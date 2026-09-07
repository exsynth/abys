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
    abys_dumper_tmp4 = clocks[1'b1];
    abys_builder_tmp0 = abys_dumper_tmp4;
  end
  always @(*)   begin
    logic abys_dumper_tmp4;
    abys_dumper_tmp4 = resets[1'b1];
    abys_builder_tmp1 = abys_dumper_tmp4;
  end
  always @(*)   begin
    logic abys_dumper_tmp4;
    abys_dumper_tmp4 = clocks[1'b0];
    abys_builder_tmp2 = abys_dumper_tmp4;
  end
  always @(*)   begin
    logic abys_dumper_tmp4;
    abys_dumper_tmp4 = resets[1'b0];
    abys_builder_tmp3 = abys_dumper_tmp4;
  end
  always @(posedge abys_builder_tmp0 or negedge abys_builder_tmp1) begin
    if (!abys_builder_tmp1) begin
      begin
        q[1'b0] <= 1'b0;
      end
    end else begin
      begin
        logic abys_dumper_tmp10;
        abys_dumper_tmp10 = d[1'b0];
        q[1'b0] <= abys_dumper_tmp10;
      end
    end
  end
  always @(posedge abys_builder_tmp2 or negedge abys_builder_tmp3) begin
    if (!abys_builder_tmp3) begin
      begin
        q[1'b1] <= 1'b0;
      end
    end else begin
      begin
        logic abys_dumper_tmp10;
        abys_dumper_tmp10 = d[1'b1];
        q[1'b1] <= abys_dumper_tmp10;
      end
    end
  end
endmodule
