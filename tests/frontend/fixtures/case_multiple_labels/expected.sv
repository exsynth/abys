module top (
  input [2:0] selector,
  input [7:0] a,
  input [7:0] b,
  output  logic [7:0] y);



  always @(*)   begin
      logic [7:0] abys_dumper_tmp11;
      abys_dumper_tmp11 = (a ^ b);
    case (selector)
    3'b0, 3'b10: begin
      y = a;
    end
    3'b1, 3'b11: begin
      y = b;
    end
    default: begin
      y = abys_dumper_tmp11;
    end
    endcase
  end
endmodule
