module top(
  input logic [7:0] a,
  output logic [7:0] y
);
  function automatic logic [7:0] invert(input logic [7:0] value);
    invert = ~value;
  endfunction

  function automatic logic [7:0] invert_twice(input logic [7:0] value);
    invert_twice = invert(invert(value));
  endfunction

  assign y = invert_twice(a);
endmodule
