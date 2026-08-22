module top
    (input logic [7 : 0] values[0 : 3],
     input logic [1 : 0] index,
     input logic [7 : 0] update0,
     input logic [7 : 0] update1,
     input logic condition,
     input logic [1 : 0] selector,
     output logic [7 : 0] updated[0 : 3]);
  always_comb begin
    updated = values;
    if (condition) begin
      updated[index] = update0;
      updated[1] = update1;
    end else begin
      updated[2] = update0;
      updated[index] = update1;
    end
    case (selector)
      2'd0: updated[index] = update0;
      2'd1: begin
        updated[index] = update1;
        updated[0] = update0;
      end
      default: updated[3] = update1;
    endcase
    updated[2] = update0;
    updated[index] = update1;
    updated[0] = update1;
  end
endmodule
