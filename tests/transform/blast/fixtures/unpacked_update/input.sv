module top(
  input  logic [7:0] values [0:3],
  input  logic [1:0] index,
  input  logic [7:0] update,
  input  logic       enable,
  output logic [7:0] updated [0:3]
);
  always_comb begin
    updated = values;
    if (enable) begin
      updated[index] = update;
    end
  end
endmodule
