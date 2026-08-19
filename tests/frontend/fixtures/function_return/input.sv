module top (
  input  logic [7:0] d_i,
  output logic [7:0] d_o
);
  function automatic logic [7:0] reverse(logic [7:0] value);
    logic [7:0] result;
    result[7:4] = {value[0], value[1], value[2], value[3]};
    result[3:0] = {value[4], value[5], value[6], value[7]};
    return result;
  endfunction

  assign d_o = reverse(d_i);
endmodule
