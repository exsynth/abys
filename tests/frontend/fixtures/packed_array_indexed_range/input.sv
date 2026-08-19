module top (
  input  logic [2:0][7:0] data_i,
  input  logic [0:2][7:0] ascending_i,
  input  logic [1:0][7:0] update_i,
  input  logic              base_i,
  output logic [15:0]       up_o,
  output logic [15:0]       down_o,
  output logic [15:0]       ascending_up_o,
  output logic [15:0]       ascending_down_o,
  output logic [2:0][7:0]   updated_up_o,
  output logic [2:0][7:0]   updated_down_o
);
  assign up_o = data_i[base_i +: 2];
  assign down_o = data_i[base_i + 1 -: 2];
  assign ascending_up_o = ascending_i[base_i +: 2];
  assign ascending_down_o = ascending_i[base_i + 1 -: 2];

  always_comb begin
    updated_up_o = data_i;
    updated_up_o[base_i +: 2] = update_i;
    updated_down_o = data_i;
    updated_down_o[base_i + 1 -: 2] = update_i;
  end
endmodule
