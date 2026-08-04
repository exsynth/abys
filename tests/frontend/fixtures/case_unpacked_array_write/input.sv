module top(
  input  logic [1:0] select,
  input  logic [7:0] a,
  output logic [7:0] y
);
  logic [7:0] memory [0:2];
  always_comb begin
    memory[0] = 8'h11;
    memory[1] = 8'h22;
    memory[2] = 8'h33;
    case (select)
      2'd0: memory[0] = a;
      2'd1: memory[1] = a;
      default: memory[2] = a;
    endcase
    y = memory[1];
  end
endmodule
