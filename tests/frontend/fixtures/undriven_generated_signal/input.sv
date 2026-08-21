module top(
  input logic select,
  output logic result
);
  logic generated_value;

  if (1'b0) begin : disabled
    assign generated_value = 1'b1;
  end

  assign result = select & generated_value;
endmodule
