module top (
  output logic [15:0] y_o
);
  always_comb begin
    logic bit_value;
    y_o = '0;
    for (int unsigned i = 0; i < 16; i = i + 1) begin
      bit_value = 1'(i);
      if (bit_value) begin
        y_o[i] = 1'b1;
      end
    end
  end
endmodule
