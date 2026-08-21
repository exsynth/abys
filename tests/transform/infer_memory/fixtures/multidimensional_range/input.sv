module top(
  input logic clk,
  input logic [1:0] write_row,
  input logic [1:0] write_column,
  input logic [1:0] read_row,
  input logic [1:0] read_base,
  input logic read_offset,
  input logic [7:0] write_data,
  output logic [7:0] read_data
);
  logic [7:0] memory [0:3][0:3];

  function automatic logic [7:0] select_from_range(
    input logic [7:0] values [0:1],
    input logic select
  );
    select_from_range = values[select];
  endfunction

  always_ff @(posedge clk) begin
    memory[write_row][write_column] <= write_data;
  end

  assign read_data = select_from_range(memory[read_row][read_base +: 2], read_offset);
endmodule
