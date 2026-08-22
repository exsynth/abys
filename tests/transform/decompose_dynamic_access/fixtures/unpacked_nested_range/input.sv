module top
    (input logic [7 : 0] values[0 : 3][0 : 3],
     input logic [1 : 0] index,
     input logic signed [2 : 0] inner_index,
     input logic [7 : 0] update_pair[0 : 1],
     input logic [7 : 0] update_element,
     output logic [7 : 0] updated_inner_range[0 : 3][0 : 3],
     output logic [7 : 0] updated_outer_range[0 : 3][0 : 3]);
  logic [7 : 0] selected_range[0 : 1][0 : 3];

  always_comb begin
    updated_inner_range = values;
    updated_inner_range[index][inner_index +: 2] = update_pair;

    updated_outer_range = values;
    selected_range = updated_outer_range[index +: 2];
    selected_range[0][inner_index] = update_element;
    updated_outer_range[index +: 2] = selected_range;
  end
endmodule
