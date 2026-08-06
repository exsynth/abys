module top(
  input logic value,
  input logic enable,
  output logic y
);
  function automatic logic filter(input logic argument);
    filter = argument & enable;
  endfunction

  assign y = filter(value);
endmodule
