module left(
  input logic [7:0] a,
  output logic [7:0] y
);
  function automatic logic [7:0] transform(input logic [7:0] value);
    transform = ~value;
  endfunction

  assign y = transform(a);
endmodule

module right(
  input logic [7:0] a,
  output logic [7:0] y
);
  function automatic logic [7:0] transform(input logic [7:0] value);
    transform = value + 8'd1;
  endfunction

  assign y = transform(a);
endmodule

module top(
  input logic [7:0] a,
  output logic [7:0] left_y,
  output logic [7:0] right_y
);
  left left_inst(.a(a), .y(left_y));
  right right_inst(.a(a), .y(right_y));
endmodule
