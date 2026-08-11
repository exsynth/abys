module top (
  output logic [7:0] y
);
  assign y = accumulate(3);

  function automatic logic [7:0] accumulate(input int unsigned count);
    logic [7:0] result = '0;
    for (int unsigned i = 0; i < count; i = i + 1)
      result = result + 1'b1;
    return result;
  endfunction
endmodule
