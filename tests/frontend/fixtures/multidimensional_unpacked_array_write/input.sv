module top(
  input  logic       row,
  input  logic       column,
  input  logic [7:0] value,
  output logic [7:0] y
);
  logic [7:0] memory [0:1][0:1];
  always_comb begin
    memory[0][0] = 8'h11;
    memory[0][1] = 8'h22;
    memory[1][0] = 8'h33;
    memory[1][1] = 8'h44;
    memory[row][column] = value;
    y = memory[1][0];
  end
endmodule
