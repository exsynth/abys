module top
    (input logic [7 : 0] values[0 : 2][0 : 2],
     input logic [1 : 0] index,
     input logic [1 : 0] inner_index,
     input logic [7 : 0] update[0 : 2],
     input logic [7 : 0] update_element,
     output logic [7 : 0] selected[0 : 2],
     output logic [7 : 0] updated[0 : 2][0 : 2],
     output logic [7 : 0] updated_nested[0 : 2][0 : 2]);
  always_comb begin
    selected = values[index];
    updated = values;
    updated[index] = update;
    updated_nested = values;
    updated_nested[index][inner_index] = update_element;
  end
endmodule
