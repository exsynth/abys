module top
    (input logic [0 : 3][7 : 0] values,
     input logic [0 : 31] flat_values,
     input logic [4 : 0] flat_index,
     input logic [1 : 0] index,
     input logic [7 : 0] update,
     input logic [1 : 0][7 : 0] update_pair,
     output logic selected_bit,
     output logic [7 : 0] selected,
     output logic [7 : 0] selected_reversed,
     output logic [1 : 0][7 : 0] selected_pair,
     output logic [0 : 31] updated_bit,
     output logic [0 : 3][7 : 0] updated,
     output logic [0 : 3][7 : 0] updated_reversed,
     output logic [0 : 3][7 : 0] updated_pair);
  always_comb begin
    selected_bit = flat_values[flat_index];
    selected = values[index];
    selected_reversed = values[3 - index];
    selected_pair = values[index +: 2];
    updated_bit = flat_values;
    updated_bit[flat_index] = update[0];
    updated = values;
    updated[index] = update;
    updated_reversed = values;
    updated_reversed[3 - index] = update;
    updated_pair = values;
    updated_pair[index +: 2] = update_pair;
  end
endmodule
