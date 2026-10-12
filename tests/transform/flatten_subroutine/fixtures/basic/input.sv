module top(input logic [7:0] a, input logic enable, output logic [7:0] y);
  function automatic logic [7:0] invert(input logic [7:0] value);
    invert = enable ? ~value : value;
  endfunction
  assign y = invert(a);
endmodule
