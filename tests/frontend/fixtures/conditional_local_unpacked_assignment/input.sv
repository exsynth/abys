module top (
  input  logic       select_i,
  input  logic [3:0] data_i,
  output logic [3:0] data_o
);
  always_comb begin
    logic [3:0] temporary [2];
    if (select_i)
      temporary[0] = data_i;
    data_o = select_i ? temporary[0] : '0;
  end
endmodule
