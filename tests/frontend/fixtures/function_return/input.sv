module top (
  input  logic [7:0] d_i,
  output logic [7:0] d_o
);
  function automatic logic [7:0] reverse(logic [7:0] value);
    return {value[0], value[1], value[2], value[3],
            value[4], value[5], value[6], value[7]};
  endfunction

  assign d_o = reverse(d_i);
endmodule
