module top (
  input  struct packed { logic flag; logic [2:0] data; } value_i,
  output struct packed { logic flag; logic [2:0] data; } value_o,
  output logic flag_o,
  output logic [2:0] data_o
);
  assign flag_o = value_i.flag;
  assign data_o = value_i.data;
  assign value_o.flag = flag_o;
  assign value_o.data = data_o;
endmodule
