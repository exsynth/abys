module top
    (input logic [7 : 0] values[0 : 3],
     input logic [1 : 0] index,
     input logic signed [2 : 0] signed_index,
     input logic [7 : 0] update,
     input logic [7 : 0] update_pair[0 : 1],
     output logic [7 : 0] selected,
     output logic [7 : 0] selected_range[0 : 1],
     output logic [7 : 0] updated[0 : 3],
     output logic [7 : 0] updated_range[0 : 3],
     output logic [7 : 0] updated_negative_range[0 : 3],
     output logic [7 : 0] concatenated[0 : 3]);
  always_comb begin
    selected = values[index];
    selected_range = values[index +: 2];
    updated = values;
    updated[index] = update;
    updated_range = values;
    updated_range[index +: 2] = update_pair;
    updated_negative_range = values;
    updated_negative_range[signed_index +: 2] = update_pair;
    concatenated = {values[0:0], values[1:3]};
  end
endmodule
