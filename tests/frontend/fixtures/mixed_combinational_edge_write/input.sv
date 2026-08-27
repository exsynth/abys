module top (
    input  logic       clk,
    input  logic       comb_data,
    input  logic       edge_data,
    output logic [1:0] q
);
  always_comb begin
    q[1] = comb_data;
  end

  always_ff @(posedge clk) begin
    q[0] <= edge_data;
  end
endmodule
