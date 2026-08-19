module top (
  input  logic signed [3:0] signed_i,
  input  logic        [3:0] unsigned_i,
  output logic signed [3:0] signed_o,
  output logic        [3:0] unsigned_o
);
  assign signed_o = $signed(unsigned_i);
  assign unsigned_o = $unsigned(signed_i);
endmodule
