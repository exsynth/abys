module top (
  input [1:0] select_i,
  output  logic [1:0] value_o);


function automatic [1:0] select_value (
  input [1:0] select
);
  begin
    case (select)
    2'b0: begin
      select_value = 2'b10;
    end
    2'b1: begin
      select_value = 2'b11;
    end
    default: begin
      select_value = 2'b1;
    end
    endcase
  end
endfunction


  always @(*)   begin
    logic [1:0] abys_dumper_tmp3;
    abys_dumper_tmp3 = select_value(select_i);
    value_o = abys_dumper_tmp3;
  end
endmodule
