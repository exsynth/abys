module top(
  input  logic [7:0] values [0:3],
  input  logic [7:0] first,
  input  logic [7:0] second,
  input  logic [7:0] third,
  input  logic       enable,
  output logic [7:0] updated [0:3]
);
  always_comb begin
    updated = values;
    updated[1] = first;
    if (enable) begin
      updated[1] = second;
    end
    updated[3] = third;
  end
endmodule
