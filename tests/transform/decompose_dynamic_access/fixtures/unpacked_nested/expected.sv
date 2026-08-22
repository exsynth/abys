module top (
  input [7:0] values [0:2] [0:2],
  input [1:0] index,
  input [1:0] inner_index,
  input [7:0] update [0:2],
  input [7:0] update_element,
  output  logic [7:0] selected [0:2],
  output  logic [7:0] updated [0:2] [0:2],
  output  logic [7:0] updated_nested [0:2] [0:2]);



  always @(*)   begin
    logic abys_dumper_tmp3;
      logic abys_dumper_tmp5;
        logic [7:0] abys_dumper_tmp8 [0:2];
        logic [7:0] abys_dumper_tmp9;
      logic abys_dumper_tmp11;
        logic [7:0] abys_dumper_tmp12;
      logic abys_dumper_tmp15;
        logic [7:0] abys_dumper_tmp17;
    logic abys_dumper_tmp21;
      logic abys_dumper_tmp22;
        logic [7:0] abys_dumper_tmp23 [0:2];
        logic [7:0] abys_dumper_tmp24;
      logic abys_dumper_tmp26;
        logic [7:0] abys_dumper_tmp27;
      logic abys_dumper_tmp30;
        logic [7:0] abys_dumper_tmp32;
    logic abys_dumper_tmp37;
      logic abys_dumper_tmp38;
        logic [7:0] abys_dumper_tmp40 [0:2];
        logic [7:0] abys_dumper_tmp41;
      logic abys_dumper_tmp43;
        logic [7:0] abys_dumper_tmp44;
      logic abys_dumper_tmp47;
        logic [7:0] abys_dumper_tmp49;
    logic abys_dumper_tmp54;
      logic [7:0] abys_dumper_tmp56;
      logic [7:0] abys_dumper_tmp57;
      logic [7:0] abys_dumper_tmp59;
      logic [7:0] abys_dumper_tmp61 [0:2];
    logic abys_dumper_tmp63;
      logic [7:0] abys_dumper_tmp64;
      logic [7:0] abys_dumper_tmp65;
      logic [7:0] abys_dumper_tmp67;
      logic [7:0] abys_dumper_tmp69 [0:2];
    logic abys_dumper_tmp72;
      logic [7:0] abys_dumper_tmp73;
      logic [7:0] abys_dumper_tmp74;
      logic [7:0] abys_dumper_tmp76;
      logic [7:0] abys_dumper_tmp79 [0:2];
    logic abys_dumper_tmp82;
    logic abys_dumper_tmp83;
    logic abys_dumper_tmp84;
    logic abys_dumper_tmp85;
    logic abys_dumper_tmp86;
    logic [7:0] abys_dumper_tmp101 [0:2];
      logic [7:0] abys_dumper_tmp87 [0:2];
      logic abys_dumper_tmp88;
        logic [7:0] abys_dumper_tmp89 [0:2];
        logic abys_dumper_tmp90;
        logic abys_dumper_tmp91;
          logic [7:0] abys_dumper_tmp93 [0:2];
          logic abys_dumper_tmp94;
            logic [7:0] abys_dumper_tmp96 [0:2];
    logic [7:0] abys_dumper_tmp102;
    logic [7:0] abys_dumper_tmp103;
    logic [7:0] abys_dumper_tmp105;
    abys_dumper_tmp3 = (index == 1'b0);
      abys_dumper_tmp5 = (inner_index == 1'b0);
        abys_dumper_tmp8 = values[1'b0];
        abys_dumper_tmp9 = abys_dumper_tmp8[1'b0];
      abys_dumper_tmp11 = (inner_index == 1'b1);
        abys_dumper_tmp12 = abys_dumper_tmp8[1'b1];
      abys_dumper_tmp15 = (inner_index == 2'b10);
        abys_dumper_tmp17 = abys_dumper_tmp8[2'b10];
    abys_dumper_tmp21 = (index == 1'b1);
      abys_dumper_tmp22 = (inner_index == 1'b0);
        abys_dumper_tmp23 = values[1'b1];
        abys_dumper_tmp24 = abys_dumper_tmp23[1'b0];
      abys_dumper_tmp26 = (inner_index == 1'b1);
        abys_dumper_tmp27 = abys_dumper_tmp23[1'b1];
      abys_dumper_tmp30 = (inner_index == 2'b10);
        abys_dumper_tmp32 = abys_dumper_tmp23[2'b10];
    abys_dumper_tmp37 = (index == 2'b10);
      abys_dumper_tmp38 = (inner_index == 1'b0);
        abys_dumper_tmp40 = values[2'b10];
        abys_dumper_tmp41 = abys_dumper_tmp40[1'b0];
      abys_dumper_tmp43 = (inner_index == 1'b1);
        abys_dumper_tmp44 = abys_dumper_tmp40[1'b1];
      abys_dumper_tmp47 = (inner_index == 2'b10);
        abys_dumper_tmp49 = abys_dumper_tmp40[2'b10];
    abys_dumper_tmp54 = (index == 1'b0);
      abys_dumper_tmp56 = update[1'b0];
      abys_dumper_tmp57 = update[1'b1];
      abys_dumper_tmp59 = update[2'b10];
      abys_dumper_tmp61 = values[1'b0];
    abys_dumper_tmp63 = (index == 1'b1);
      abys_dumper_tmp64 = update[1'b0];
      abys_dumper_tmp65 = update[1'b1];
      abys_dumper_tmp67 = update[2'b10];
      abys_dumper_tmp69 = values[1'b1];
    abys_dumper_tmp72 = (index == 2'b10);
      abys_dumper_tmp73 = update[1'b0];
      abys_dumper_tmp74 = update[1'b1];
      abys_dumper_tmp76 = update[2'b10];
      abys_dumper_tmp79 = values[2'b10];
    abys_dumper_tmp82 = index[1'b1];
    abys_dumper_tmp83 = (!abys_dumper_tmp82);
    abys_dumper_tmp84 = index[1'b0];
    abys_dumper_tmp85 = (!abys_dumper_tmp84);
    abys_dumper_tmp86 = (abys_dumper_tmp83 & abys_dumper_tmp85);
      abys_dumper_tmp87 = values[1'b0];
      abys_dumper_tmp88 = (abys_dumper_tmp83 & abys_dumper_tmp84);
        abys_dumper_tmp89 = values[1'b1];
        abys_dumper_tmp90 = (!abys_dumper_tmp84);
        abys_dumper_tmp91 = (abys_dumper_tmp82 & abys_dumper_tmp90);
          abys_dumper_tmp93 = values[2'b10];
          abys_dumper_tmp94 = (abys_dumper_tmp82 & abys_dumper_tmp84);
            abys_dumper_tmp96[0] = {24'bx}[0 +: 8];
            abys_dumper_tmp96[1] = {24'bx}[8 +: 8];
            abys_dumper_tmp96[2] = {24'bx}[16 +: 8];
    if (abys_dumper_tmp86) begin
      abys_dumper_tmp101 = abys_dumper_tmp87;
    end else begin
      if (abys_dumper_tmp88) begin
        abys_dumper_tmp101 = abys_dumper_tmp89;
      end else begin
        if (abys_dumper_tmp91) begin
          abys_dumper_tmp101 = abys_dumper_tmp93;
        end else begin
          if (abys_dumper_tmp94) begin
            abys_dumper_tmp101 = abys_dumper_tmp96;
          end else begin
            abys_dumper_tmp101 = selected;
          end
        end
      end
    end
    abys_dumper_tmp102 = abys_dumper_tmp101[1'b0];
    abys_dumper_tmp103 = abys_dumper_tmp101[1'b1];
    abys_dumper_tmp105 = abys_dumper_tmp101[2'b10];
    if (abys_dumper_tmp3) begin
      if (abys_dumper_tmp5) begin
        updated_nested[0][0] = update_element;
      end else begin
        updated_nested[0][0] = abys_dumper_tmp9;
      end
      if (abys_dumper_tmp11) begin
        updated_nested[0][1] = update_element;
      end else begin
        updated_nested[0][1] = abys_dumper_tmp12;
      end
      if (abys_dumper_tmp15) begin
        updated_nested[0][2] = update_element;
      end else begin
        updated_nested[0][2] = abys_dumper_tmp17;
      end
    end else begin
      updated_nested[0] = abys_dumper_tmp8;
    end
    if (abys_dumper_tmp21) begin
      if (abys_dumper_tmp22) begin
        updated_nested[1][0] = update_element;
      end else begin
        updated_nested[1][0] = abys_dumper_tmp24;
      end
      if (abys_dumper_tmp26) begin
        updated_nested[1][1] = update_element;
      end else begin
        updated_nested[1][1] = abys_dumper_tmp27;
      end
      if (abys_dumper_tmp30) begin
        updated_nested[1][2] = update_element;
      end else begin
        updated_nested[1][2] = abys_dumper_tmp32;
      end
    end else begin
      updated_nested[1] = abys_dumper_tmp23;
    end
    if (abys_dumper_tmp37) begin
      if (abys_dumper_tmp38) begin
        updated_nested[2][0] = update_element;
      end else begin
        updated_nested[2][0] = abys_dumper_tmp41;
      end
      if (abys_dumper_tmp43) begin
        updated_nested[2][1] = update_element;
      end else begin
        updated_nested[2][1] = abys_dumper_tmp44;
      end
      if (abys_dumper_tmp47) begin
        updated_nested[2][2] = update_element;
      end else begin
        updated_nested[2][2] = abys_dumper_tmp49;
      end
    end else begin
      updated_nested[2] = abys_dumper_tmp40;
    end
    if (abys_dumper_tmp54) begin
      updated[0][0] = abys_dumper_tmp56;
      updated[0][1] = abys_dumper_tmp57;
      updated[0][2] = abys_dumper_tmp59;
    end else begin
      updated[0] = abys_dumper_tmp61;
    end
    if (abys_dumper_tmp63) begin
      updated[1][0] = abys_dumper_tmp64;
      updated[1][1] = abys_dumper_tmp65;
      updated[1][2] = abys_dumper_tmp67;
    end else begin
      updated[1] = abys_dumper_tmp69;
    end
    if (abys_dumper_tmp72) begin
      updated[2][0] = abys_dumper_tmp73;
      updated[2][1] = abys_dumper_tmp74;
      updated[2][2] = abys_dumper_tmp76;
    end else begin
      updated[2] = abys_dumper_tmp79;
    end
    selected[0] = abys_dumper_tmp102;
    selected[1] = abys_dumper_tmp103;
    selected[2] = abys_dumper_tmp105;
  end
endmodule
