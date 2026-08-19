package arithmetic_pkg;
  function automatic logic [7:0] increment(input logic [7:0] value);
    increment = value + 8'd1;
  endfunction
endpackage

module top(
  input logic [7:0] value_i,
  output logic [7:0] value_o
);
  assign value_o = arithmetic_pkg::increment(value_i);
endmodule
