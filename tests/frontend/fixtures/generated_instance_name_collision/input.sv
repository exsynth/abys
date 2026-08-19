module child #(
  parameter int Width = 1
) (
  input  logic [Width-1:0] d_i,
  output logic [Width-1:0] d_o
);
  assign d_o = d_i;
endmodule

module top (
  input  logic a_i,
  output logic a_o,
  input  logic [1:0] b_i,
  output logic [1:0] b_o
);
  for (genvar i = 0; i < 1; i = i + 1) begin : gen_a
    child #(.Width(1)) i_child (.d_i(a_i), .d_o(a_o));
  end
  for (genvar i = 0; i < 1; i = i + 1) begin : gen_b
    child #(.Width(2)) i_child (.d_i(b_i), .d_o(b_o));
  end
endmodule
