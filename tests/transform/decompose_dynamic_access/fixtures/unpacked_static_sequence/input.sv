module top
    (input logic [7 : 0] values[0 : 3],
     input logic [7 : 0] update0,
     input logic [7 : 0] update1,
     input logic [7 : 0] update2,
     output logic [7 : 0] updated[0 : 3]);
  always_comb begin
    updated = values;
    updated[2] = update0;
    updated[0] = update1;
    updated[2] = update2;
  end
endmodule
