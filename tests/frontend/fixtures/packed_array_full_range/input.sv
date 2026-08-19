module top (
  input  logic [2:0][7:0] data_i,
  output logic [23:0]      simple_o,
  output logic [23:0]      indexed_o
);
  assign simple_o = data_i[2:0];
  assign indexed_o = data_i[0 +: 3];
endmodule
