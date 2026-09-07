module top(
  input logic clk,
  input logic [2:0] write_index,
  input logic [2:0] read_base,
  input logic [7:0] write_data,
  output logic [7:0] read_data
);
  logic [7:0] memory [0:7];

  function automatic logic [7:0] select_only_element(input logic [7:0] values [0:0]);
    select_only_element = values[0];
  endfunction

  always_ff @(posedge clk) begin
    memory[write_index +: 1] <= '{write_data};
  end

  assign read_data = select_only_element(memory[read_base +: 1]);
endmodule
