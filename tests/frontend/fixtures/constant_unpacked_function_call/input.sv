module top (
  output logic [3:0] y
);
  typedef logic [3:0] array_t [2];

  array_t values;
  assign values = make_values(1);
  assign y = values[0];

  function automatic array_t make_values(input int unsigned count);
    array_t result;
    for (int unsigned i = 0; i < count; i = i + 1)
      result[i] = 4'ha;
    result[1] = 4'h5;
    return result;
  endfunction
endmodule
