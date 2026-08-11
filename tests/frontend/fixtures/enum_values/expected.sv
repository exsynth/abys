module top (
  input select_i,
  output  logic [1:0] state_o);

  logic [1:0] state;


  always @(*)   begin
    logic [1:0] abys_dumper_tmp5;
    if (select_i) begin
      abys_dumper_tmp5 = 2'b1;
    end else begin
      abys_dumper_tmp5 = 2'b0;
    end
    case (abys_dumper_tmp5)
    2'b1: begin
      state_o = 2'b10;
    end
    default: begin
      state_o = 2'b0;
    end
    endcase
    if (select_i) begin
      state = 2'b1;
    end else begin
      state = 2'b0;
    end
  end
endmodule
