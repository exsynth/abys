module top (
  input [1:0] select,
  input [7:0] a,
  output  logic [7:0] y);

  logic [7:0] memory [2:0];


  always @(*)   begin
    logic [7:0] abys_dumper_tmp47 [2:0];
    logic signed [32:0] abys_dumper_tmp4;
    logic signed [32:0] abys_dumper_tmp6;
    logic signed [32:0] abys_dumper_tmp12;
    logic signed [32:0] abys_dumper_tmp14;
    logic signed [32:0] abys_dumper_tmp19;
    logic signed [32:0] abys_dumper_tmp21;
      logic signed [32:0] abys_dumper_tmp28;
      logic signed [32:0] abys_dumper_tmp30;
      logic signed [32:0] abys_dumper_tmp35;
      logic signed [32:0] abys_dumper_tmp37;
      logic signed [32:0] abys_dumper_tmp41;
      logic signed [32:0] abys_dumper_tmp43;
    logic signed [32:0] abys_dumper_tmp49;
    logic signed [32:0] abys_dumper_tmp51;
    logic [7:0] abys_dumper_tmp52;
    abys_dumper_tmp47 = memory;
    abys_dumper_tmp4 = 32'sb0;
    abys_dumper_tmp6 = (33'sb10 - abys_dumper_tmp4);
    abys_dumper_tmp47[abys_dumper_tmp6] = 8'b10001;
    abys_dumper_tmp12 = 32'sb1;
    abys_dumper_tmp14 = (33'sb10 - abys_dumper_tmp12);
    abys_dumper_tmp47[abys_dumper_tmp14] = 8'b100010;
    abys_dumper_tmp19 = 32'sb10;
    abys_dumper_tmp21 = (33'sb10 - abys_dumper_tmp19);
    abys_dumper_tmp47[abys_dumper_tmp21] = 8'b110011;
      abys_dumper_tmp28 = 32'sb0;
      abys_dumper_tmp30 = (33'sb10 - abys_dumper_tmp28);
      abys_dumper_tmp35 = 32'sb1;
      abys_dumper_tmp37 = (33'sb10 - abys_dumper_tmp35);
      abys_dumper_tmp41 = 32'sb10;
      abys_dumper_tmp43 = (33'sb10 - abys_dumper_tmp41);
    case (select)
    2'b0: begin
      abys_dumper_tmp47[abys_dumper_tmp6] = 8'b10001;
      abys_dumper_tmp47[abys_dumper_tmp14] = 8'b100010;
      abys_dumper_tmp47[abys_dumper_tmp21] = 8'b110011;
      abys_dumper_tmp47[abys_dumper_tmp30] = a;
    end
    2'b1: begin
      abys_dumper_tmp47[abys_dumper_tmp6] = 8'b10001;
      abys_dumper_tmp47[abys_dumper_tmp14] = 8'b100010;
      abys_dumper_tmp47[abys_dumper_tmp21] = 8'b110011;
      abys_dumper_tmp47[abys_dumper_tmp37] = a;
    end
    default: begin
      abys_dumper_tmp47[abys_dumper_tmp6] = 8'b10001;
      abys_dumper_tmp47[abys_dumper_tmp14] = 8'b100010;
      abys_dumper_tmp47[abys_dumper_tmp21] = 8'b110011;
      abys_dumper_tmp47[abys_dumper_tmp43] = a;
    end
    endcase
    abys_dumper_tmp49 = 32'sb1;
    abys_dumper_tmp51 = (33'sb10 - abys_dumper_tmp49);
    abys_dumper_tmp52 = abys_dumper_tmp47[abys_dumper_tmp51];
    y = abys_dumper_tmp52;
    memory[abys_dumper_tmp6] = 8'b10001;
    memory[abys_dumper_tmp14] = 8'b100010;
    memory[abys_dumper_tmp21] = 8'b110011;
    case (select)
    2'b0: begin
      memory[abys_dumper_tmp6] = 8'b10001;
      memory[abys_dumper_tmp14] = 8'b100010;
      memory[abys_dumper_tmp21] = 8'b110011;
      memory[abys_dumper_tmp30] = a;
    end
    2'b1: begin
      memory[abys_dumper_tmp6] = 8'b10001;
      memory[abys_dumper_tmp14] = 8'b100010;
      memory[abys_dumper_tmp21] = 8'b110011;
      memory[abys_dumper_tmp37] = a;
    end
    default: begin
      memory[abys_dumper_tmp6] = 8'b10001;
      memory[abys_dumper_tmp14] = 8'b100010;
      memory[abys_dumper_tmp21] = 8'b110011;
      memory[abys_dumper_tmp43] = a;
    end
    endcase
  end
endmodule
