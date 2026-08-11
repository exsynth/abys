module top #(
  parameter int Width = 4
) (
  input  logic [Width-1:0] d_i,
  output logic [Width-1:0] d_o
);
  if (Width == 1) begin : gen_single
    assign d_o = ~d_i;
  end else begin : gen_multiple
    assign d_o = d_i;
  end
endmodule
