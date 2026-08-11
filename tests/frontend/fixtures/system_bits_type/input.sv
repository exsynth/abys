module top (
  output logic [31:0] width_o
);
  typedef struct packed {
    logic       flag;
    logic [6:0] data;
  } value_t;

  assign width_o = $bits(value_t);
endmodule
