module top #(
  parameter int Width = 16,
  parameter int UsedWidth = 16
) (
  input  logic [UsedWidth-1:0] d_i,
  output logic [Width-1:0] d_o
);
  assign d_o = {{(Width-UsedWidth){1'b0}}, d_i};
endmodule
