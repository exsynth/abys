module top(
  input logic clk,
  input logic [2:0] write_index,
  input logic [2:0] read_base,
  input logic offset,
  input logic [7:0] write_data,
  output logic [7:0] read_data
);
  logic [7:0] memory [0:7];

  function automatic logic [7:0] select_from_range(
    input logic [7:0] values [0:1],
    input logic select
  );
    select_from_range = values[select];
  endfunction

  always_ff @(posedge clk) begin
    memory[write_index] <= write_data;
  end

  assign read_data = select_from_range(memory[read_base +: 2], offset);
endmodule
