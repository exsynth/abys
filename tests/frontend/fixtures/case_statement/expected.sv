module top (
  input [1:0] select,
  input [7:0] a,
  input [7:0] b,
  output  logic [7:0] y);



  always @(*)   begin
    case (select)
    2'b0: begin
      y = a;
    end
    2'b1: begin
      y = b;
    end
    default: begin
      y = 8'b0;
    end
    endcase
  end
endmodule
