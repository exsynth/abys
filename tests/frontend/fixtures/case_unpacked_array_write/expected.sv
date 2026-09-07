module top (
  input [1:0] select,
  input [7:0] a,
  output  logic [7:0] y);

  logic [7:0] memory [0:2];


  always @(*)   begin
    logic [7:0] abys_dumper_tmp29 [0:2];
    logic [7:0] abys_dumper_tmp31;
    abys_dumper_tmp29 = memory;
    abys_dumper_tmp29[32'sb0] = 8'b10001;
    abys_dumper_tmp29[32'sb1] = 8'b100010;
    abys_dumper_tmp29[32'sb10] = 8'b110011;
    case (select)
    2'b0: begin
      abys_dumper_tmp29[32'sb0] = 8'b10001;
      abys_dumper_tmp29[32'sb1] = 8'b100010;
      abys_dumper_tmp29[32'sb10] = 8'b110011;
      abys_dumper_tmp29[32'sb0] = a;
    end
    2'b1: begin
      abys_dumper_tmp29[32'sb0] = 8'b10001;
      abys_dumper_tmp29[32'sb1] = 8'b100010;
      abys_dumper_tmp29[32'sb10] = 8'b110011;
      abys_dumper_tmp29[32'sb1] = a;
    end
    default: begin
      abys_dumper_tmp29[32'sb0] = 8'b10001;
      abys_dumper_tmp29[32'sb1] = 8'b100010;
      abys_dumper_tmp29[32'sb10] = 8'b110011;
      abys_dumper_tmp29[32'sb10] = a;
    end
    endcase
    abys_dumper_tmp31 = abys_dumper_tmp29[32'sb1];
    y = abys_dumper_tmp31;
    memory[32'sb0] = 8'b10001;
    memory[32'sb1] = 8'b100010;
    memory[32'sb10] = 8'b110011;
    case (select)
    2'b0: begin
      memory[32'sb0] = 8'b10001;
      memory[32'sb1] = 8'b100010;
      memory[32'sb10] = 8'b110011;
      memory[32'sb0] = a;
    end
    2'b1: begin
      memory[32'sb0] = 8'b10001;
      memory[32'sb1] = 8'b100010;
      memory[32'sb10] = 8'b110011;
      memory[32'sb1] = a;
    end
    default: begin
      memory[32'sb0] = 8'b10001;
      memory[32'sb1] = 8'b100010;
      memory[32'sb10] = 8'b110011;
      memory[32'sb10] = a;
    end
    endcase
  end
endmodule
