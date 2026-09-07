module top(
  input logic clk,
  input logic write_enable,
  input logic [1:0] write_index,
  input logic [1:0] read_index,
  input logic [7:0] write_data,
  output logic [7:0] read_data_if,
  output logic [7:0] read_data_else
);
  logic [7:0] memory_if [3:0];
  logic [7:0] memory_else [3:0];

  always_ff @(posedge clk) begin
    if (write_enable) begin
      memory_if[write_index] <= write_data;
    end
  end

  always_ff @(posedge clk) begin
    if (write_enable) begin
    end else begin
      memory_else[write_index] <= write_data;
    end
  end

  assign read_data_if = memory_if[read_index];
  assign read_data_else = memory_else[read_index];
endmodule
