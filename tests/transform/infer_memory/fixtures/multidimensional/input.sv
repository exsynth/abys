module top(
  input logic clk,
  input logic [1:0] write_row,
  input logic [1:0] write_column,
  input logic [1:0] read_row,
  input logic [1:0] read_column,
  input logic [7:0] write_data,
  output logic [7:0] read_data
);
  logic [7:0] memory [0:3][0:3];

  always_ff @(posedge clk) begin
    memory[write_row][write_column] <= write_data;
  end

  assign read_data = memory[read_row][read_column];
endmodule
