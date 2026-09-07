module top
    (input logic [7 : 0] values[-2 : 2],
     input logic signed [3 : 0] index,
     input logic [3 : 0] unsigned_index,
     input logic [7 : 0] update,
     input logic [7 : 0] update_pair[0 : 1],
     output logic [7 : 0] selected,
     output logic [7 : 0] selected_up[0 : 1],
     output logic [7 : 0] selected_down[0 : 1],
     output logic [7 : 0] selected_unsigned,
     output logic [7 : 0] selected_unsigned_range[0 : 1],
     output logic [7 : 0] updated_select[-2 : 2],
     output logic [7 : 0] updated_up[-2 : 2],
     output logic [7 : 0] updated_down[-2 : 2],
     output logic [7 : 0] updated_unsigned_range[-2 : 2]);
  always_comb begin
    selected = values[index];
    selected_up = values[index +: 2];
    selected_down = values[index -: 2];
    selected_unsigned = values[unsigned_index];
    selected_unsigned_range = values[unsigned_index +: 2];

    updated_select = values;
    updated_select[index] = update;

    updated_up = values;
    updated_up[index +: 2] = update_pair;

    updated_down = values;
    updated_down[index -: 2] = update_pair;

    updated_unsigned_range = values;
    updated_unsigned_range[unsigned_index +: 2] = update_pair;
  end
endmodule
