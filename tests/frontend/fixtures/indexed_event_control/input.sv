module top(
  input logic [1:0] clocks,
  input logic [2:3] resets,
  input logic [1:0] d,
  output logic [1:0] q
);
  always_ff @(posedge clocks[1] or negedge resets[2]) begin
    if (!resets[2]) begin
      q[0] <= 1'b0;
    end else begin
      q[0] <= d[0];
    end
  end

  always_ff @(posedge clocks[0] or negedge resets[3]) begin
    if (!resets[3]) begin
      q[1] <= 1'b0;
    end else begin
      q[1] <= d[1];
    end
  end
endmodule
