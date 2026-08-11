module top (
  input  logic       select_i,
  output logic [1:0] state_o
);
  enum logic [1:0] {
    IDLE,
    ACTIVE,
    DONE
  } state;

  always_comb begin
    if (select_i) begin
      state = ACTIVE;
    end else begin
      state = IDLE;
    end

    case (state)
      ACTIVE: state_o = DONE;
      default: state_o = IDLE;
    endcase
  end
endmodule
