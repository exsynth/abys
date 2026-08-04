module top(
  input  logic [1:0] index,
  input  logic [7:0] value,
  output logic [7:0] y
);
  logic [7:0] memory [0:3];
  always_comb begin
    memory[0] = 8'h11;
    memory[1] = 8'h22;
    memory[2] = 8'h33;
    memory[3] = 8'h44;
    memory[index] = value;
    y = memory[2];
  end
endmodule
