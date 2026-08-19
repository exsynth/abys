module top (
  input  logic [3:0][7:0] data_i,
  input  logic [1:0]      index_i,
  input  logic [7:0]      update_i,
  output logic [7:0]      selected_o,
  output logic [3:0][7:0] updated_o
);
  assign selected_o = data_i[index_i];

  always_comb begin
    updated_o = data_i;
    updated_o[index_i] = update_i;
  end
endmodule
