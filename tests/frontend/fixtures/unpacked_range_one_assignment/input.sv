module top(
  input logic [7:0] values [0:0],
  output logic [7:0] result
);
  logic [7:0] updated [0:0];

  always_comb begin
    updated[0 +: 1] = values[0 +: 1];
    result = updated[0];
  end
endmodule
