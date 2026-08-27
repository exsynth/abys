module top(
  input  logic [7:0] values [0:3],
  input  logic [7:0] update [0:1],
  input  logic [1:0] base,
  output logic [7:0] updated [0:3]
);
  always_comb begin
    updated = values;
    updated[base +: 2] = update;
  end
endmodule
