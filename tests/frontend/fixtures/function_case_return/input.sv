module top (
  input logic [1:0] select_i,
  output logic [1:0] value_o
);
  function automatic logic [1:0] select_value(input logic [1:0] select);
    case (select)
      2'b00: return 2'b10;
      2'b01: return 2'b11;
      default: return 2'b01;
    endcase
  endfunction

  assign value_o = select_value(select_i);
endmodule
