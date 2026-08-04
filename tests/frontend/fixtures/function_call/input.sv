module top(
  input logic [7:0] a,
  output logic [7:0] y
);
  function automatic logic [7:0] invert(input logic [7:0] value);
    invert = ~value;
  endfunction

  assign y = invert(a);
endmodule
