module child(input logic [7:0] a, output logic [7:0] y);
  assign y = ~a;
endmodule

module top(input logic [7:0] a, output logic [7:0] y);
  child u_child(.a(a), .y(y));
endmodule
