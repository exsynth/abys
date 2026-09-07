module child (
  input  logic [1:0] data_i,
  output logic [1:0] data_o
);
  assign data_o = data_i;
endmodule

module top (
  input  logic [1:0] data_i [3:0],
  output logic [1:0] data_o [3:0]
);
  child child_0 (.data_i(data_i[0]), .data_o(data_o[0]));
  child child_1 (.data_i(data_i[1]), .data_o(data_o[1]));
  child child_2 (.data_i(data_i[2]), .data_o(data_o[2]));
  child child_3 (.data_i(data_i[3]), .data_o(data_o[3]));
endmodule
