module top(
  input  logic       select,
  input  logic [7:0] a,
  input  logic [7:0] b,
  output logic [7:0] y
);
  logic [7:0] memory [0:1];
  always_comb begin
    memory[0] = a;
    memory[1] = b;
    if (select)
      memory[0] = b;
    else
      memory[1] = a;
    y = memory[0] ^ memory[1];
  end
endmodule
