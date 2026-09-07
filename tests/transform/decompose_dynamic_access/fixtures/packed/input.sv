module top
    (input logic [3 : 0][7 : 0] values,
     input logic [31 : 0] flat_values,
     input logic [0 : 3][7 : 0] ascending_values,
     input logic [1 : 0][3 : 0][7 : 0] nested_values,
     input logic [1 : 0] index,
     input logic outer_index,
     input logic [1 : 0] inner_index,
     input logic signed [5 : 0] signed_index,
     input logic [7 : 0] update,
     input logic [1 : 0][7 : 0] update_pair,
     input logic [5 : 0] update_offset,
     output logic [7 : 0] selected,
     output logic [1 : 0][7 : 0] selected_pair,
     output logic [5 : 0] selected_offset,
     output logic [7 : 0] selected_ascending,
     output logic [7 : 0] selected_nested,
     output logic [7 : 0] selected_signed,
     output logic [3 : 0][7 : 0] updated,
     output logic [3 : 0][7 : 0] updated_pair,
     output logic [31 : 0] updated_signed,
     output logic [3 : 0][7 : 0] updated_offset,
     output logic [0 : 3][7 : 0] updated_ascending,
     output logic [1 : 0][3 : 0][7 : 0] updated_nested);
  always_comb begin
    selected = values[index];
    selected_pair = values[index +: 2];
    selected_offset = values[index][6 : 1];
    selected_ascending = ascending_values[index];
    selected_nested = nested_values[outer_index][inner_index];
    selected_signed = flat_values[signed_index +: 8];
    updated = values;
    updated[index] = update;
    updated_pair = values;
    updated_pair[index +: 2] = update_pair;
    updated_signed = flat_values;
    updated_signed[signed_index +: 8] = update;
    updated_offset = values;
    updated_offset[index][6 : 1] = update_offset;
    updated_ascending = ascending_values;
    updated_ascending[index] = update;
    updated_nested = nested_values;
    updated_nested[outer_index][inner_index] = update;
  end
endmodule
