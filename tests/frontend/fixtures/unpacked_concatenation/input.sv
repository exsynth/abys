module top
    (input logic [7 : 0] values[0 : 3],
     output logic [7 : 0] concatenated[0 : 3]);
  assign concatenated = {values[0:0], values[1:3]};
endmodule
