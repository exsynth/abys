module top(
  input logic sign_i,
  input logic [2:0] data_i,
  output logic [7:0] packed_o,
  output logic [2:0] array_o [0:2]
);
  typedef struct packed {
    logic sign;
    logic [3:0] exponent;
    logic [2:0] mantissa;
  } value_t;

  value_t value;
  logic [2:0] array [0:2];

  assign value = '{mantissa: data_i, sign: sign_i, default: 1'b1};
  assign array = '{1: data_i, default: 3'b101};
  assign packed_o = value;
  assign array_o = array;
endmodule
