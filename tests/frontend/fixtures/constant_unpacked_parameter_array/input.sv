module top (
  input  logic [1:0] select_i,
  output logic [7:0] d_o
);
  localparam logic [7:0] Values [0:3] = '{8'h12, 8'h34, 8'h56, 8'h78};

  assign d_o = Values[select_i];
endmodule
