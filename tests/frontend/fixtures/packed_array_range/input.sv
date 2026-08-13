module top(
  input  logic [2:0][7:0] data_i,
  input  logic [1:0][7:0] update_i,
  output logic [15:0] data_o,
  output logic [2:0][7:0] updated_o
);
  assign data_o = data_i[1:0];
  always_comb begin
    updated_o = data_i;
    updated_o[1:0] = update_i;
  end
endmodule
