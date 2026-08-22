module top
    (input logic [12 : 0] data,
     input logic signed [12 : 0] signed_data,
     input logic [3 : 0] amount,
     output logic [12 : 0] left,
     output logic [12 : 0] right,
     output logic signed [12 : 0] arithmetic_right);
  assign left = data << amount;
  assign right = data >> amount;
  assign arithmetic_right = signed_data >>> amount;
endmodule
