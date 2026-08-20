module top(
  input logic clk,
  input logic write_enable,
  input logic [1:0] write_index,
  input logic [1:0] read_index,
  input logic [7:0] write_data,
  output logic [7:0] read_data
);
  logic [7:0] memory [3:0];

  always_ff @(posedge clk) begin
    if (write_enable) begin
      memory[write_index] <= write_data;
    end
  end

  assign read_data = memory[read_index];
endmodule
