module top (
  input [1:0] data_i [0:3],
  output  logic [1:0] data_o [0:3]);

  logic [1:0] abys_builder_tmp0;
  logic [1:0] abys_builder_tmp1;
  logic [1:0] abys_builder_tmp2;
  logic [1:0] abys_builder_tmp3;
  logic [1:0] abys_builder_tmp4;
  logic [1:0] abys_builder_tmp5;
  logic [1:0] abys_builder_tmp6;
  logic [1:0] abys_builder_tmp7;

  child child_0 (
    .data_i(abys_builder_tmp0),
    .data_o(abys_builder_tmp1)  );
  child child_1 (
    .data_i(abys_builder_tmp2),
    .data_o(abys_builder_tmp3)  );
  child child_2 (
    .data_i(abys_builder_tmp4),
    .data_o(abys_builder_tmp5)  );
  child child_3 (
    .data_i(abys_builder_tmp6),
    .data_o(abys_builder_tmp7)  );

  always @(*)   begin
    logic [1:0] abys_dumper_tmp4;
    abys_dumper_tmp4 = data_i[32'sb0];
    abys_builder_tmp0 = abys_dumper_tmp4;
  end
  always @(*)   begin
    logic [1:0] abys_dumper_tmp4;
    abys_dumper_tmp4 = data_i[32'sb1];
    abys_builder_tmp2 = abys_dumper_tmp4;
  end
  always @(*) begin
    begin
      data_o[32'sb0] = abys_builder_tmp1;
    end
    begin
      data_o[32'sb1] = abys_builder_tmp3;
    end
    begin
      data_o[32'sb10] = abys_builder_tmp5;
    end
    begin
      data_o[32'sb11] = abys_builder_tmp7;
    end
  end
  always @(*)   begin
    logic [1:0] abys_dumper_tmp4;
    abys_dumper_tmp4 = data_i[32'sb10];
    abys_builder_tmp4 = abys_dumper_tmp4;
  end
  always @(*)   begin
    logic [1:0] abys_dumper_tmp4;
    abys_dumper_tmp4 = data_i[32'sb11];
    abys_builder_tmp6 = abys_dumper_tmp4;
  end
endmodule

module child (
  input [1:0] data_i,
  output  logic [1:0] data_o);



  always @(*)   begin
    data_o = data_i;
  end
endmodule
