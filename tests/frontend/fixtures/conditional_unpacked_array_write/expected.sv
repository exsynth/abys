module top (
  input select,
  input [7:0] a,
  input [7:0] b,
  output  logic [7:0] y);

  logic [7:0] memory [0:1];


  always @(*)   begin
    logic [7:0] abys_dumper_tmp19 [0:1];
    logic [7:0] abys_dumper_tmp21;
    logic [7:0] abys_dumper_tmp23;
    logic [7:0] abys_dumper_tmp24;
    abys_dumper_tmp19 = memory;
    abys_dumper_tmp19[32'sb0] = a;
    abys_dumper_tmp19[32'sb1] = b;
    if (select) begin
      abys_dumper_tmp19[32'sb0] = a;
      abys_dumper_tmp19[32'sb1] = b;
      abys_dumper_tmp19[32'sb0] = b;
    end else begin
      abys_dumper_tmp19[32'sb0] = a;
      abys_dumper_tmp19[32'sb1] = b;
      abys_dumper_tmp19[32'sb1] = a;
    end
    abys_dumper_tmp21 = abys_dumper_tmp19[32'sb0];
    abys_dumper_tmp23 = abys_dumper_tmp19[32'sb1];
    abys_dumper_tmp24 = (abys_dumper_tmp21 ^ abys_dumper_tmp23);
    y = abys_dumper_tmp24;
    memory[32'sb0] = a;
    memory[32'sb1] = b;
    if (select) begin
      memory[32'sb0] = a;
      memory[32'sb1] = b;
      memory[32'sb0] = b;
    end else begin
      memory[32'sb0] = a;
      memory[32'sb1] = b;
      memory[32'sb1] = a;
    end
  end
endmodule
