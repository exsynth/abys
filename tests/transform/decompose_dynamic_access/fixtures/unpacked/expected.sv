module top (
  input [7:0] values [0:3],
  input [1:0] index,
  input signed [2:0] signed_index,
  input [7:0] update,
  input [7:0] update_pair [0:1],
  output  logic [7:0] selected,
  output  logic [7:0] selected_range [0:1],
  output  logic [7:0] updated [0:3],
  output  logic [7:0] updated_range [0:3],
  output  logic [7:0] updated_negative_range [0:3],
  output  logic [7:0] concatenated [0:3]);



  always @(*)   begin
    logic [7:0] abys_dumper_tmp3 [0:0];
    logic [7:0] abys_dumper_tmp4 [0:2];
    logic [7:0] abys_dumper_tmp5 [0:3];
    logic [7:0] abys_dumper_tmp6;
    logic [7:0] abys_dumper_tmp7;
    logic [7:0] abys_dumper_tmp9;
    logic [7:0] abys_dumper_tmp11;
    logic abys_dumper_tmp18;
    logic signed [3:0] abys_dumper_tmp14;
    logic signed [3:0] abys_dumper_tmp16;
    logic abys_dumper_tmp20;
    logic abys_dumper_tmp21;
    logic abys_dumper_tmp22;
    logic abys_dumper_tmp23;
    logic abys_dumper_tmp24;
    logic abys_dumper_tmp25;
    logic abys_dumper_tmp26;
      logic abys_dumper_tmp28;
        logic [7:0] abys_dumper_tmp30;
        logic abys_dumper_tmp32;
          logic abys_dumper_tmp33;
            logic abys_dumper_tmp34;
              logic [7:0] abys_dumper_tmp35;
      logic [7:0] abys_dumper_tmp40;
    logic abys_dumper_tmp42;
    logic abys_dumper_tmp43;
    logic abys_dumper_tmp44;
    logic abys_dumper_tmp45;
    logic abys_dumper_tmp46;
      logic [7:0] abys_dumper_tmp52;
    logic abys_dumper_tmp54;
    logic abys_dumper_tmp55;
    logic abys_dumper_tmp56;
    logic abys_dumper_tmp57;
      logic [7:0] abys_dumper_tmp63;
    logic abys_dumper_tmp65;
    logic abys_dumper_tmp66;
    logic abys_dumper_tmp67;
    logic abys_dumper_tmp68;
    logic abys_dumper_tmp69;
      logic [7:0] abys_dumper_tmp76;
    logic abys_dumper_tmp84;
    logic signed [2:0] abys_dumper_tmp80;
    logic signed [2:0] abys_dumper_tmp82;
    logic abys_dumper_tmp85;
    logic abys_dumper_tmp86;
    logic abys_dumper_tmp87;
    logic abys_dumper_tmp88;
    logic abys_dumper_tmp89;
      logic abys_dumper_tmp91;
        logic [7:0] abys_dumper_tmp92;
        logic abys_dumper_tmp93;
          logic abys_dumper_tmp94;
            logic [7:0] abys_dumper_tmp95;
      logic [7:0] abys_dumper_tmp99;
    logic abys_dumper_tmp101;
    logic abys_dumper_tmp102;
    logic abys_dumper_tmp103;
    logic abys_dumper_tmp104;
      logic [7:0] abys_dumper_tmp109;
    logic abys_dumper_tmp111;
    logic abys_dumper_tmp112;
    logic abys_dumper_tmp113;
      logic [7:0] abys_dumper_tmp118;
    logic abys_dumper_tmp120;
    logic abys_dumper_tmp121;
    logic abys_dumper_tmp122;
    logic abys_dumper_tmp123;
      logic [7:0] abys_dumper_tmp129;
    logic abys_dumper_tmp132;
      logic [7:0] abys_dumper_tmp134;
    logic abys_dumper_tmp136;
      logic [7:0] abys_dumper_tmp137;
    logic abys_dumper_tmp140;
      logic [7:0] abys_dumper_tmp142;
    logic abys_dumper_tmp145;
      logic [7:0] abys_dumper_tmp147;
    logic abys_dumper_tmp154;
    logic signed [2:0] abys_dumper_tmp150;
    logic signed [3:0] abys_dumper_tmp152;
      logic abys_dumper_tmp157;
        logic abys_dumper_tmp158;
          logic abys_dumper_tmp159;
            logic [7:0] abys_dumper_tmp161;
            logic [7:0] abys_dumper_tmp165;
            logic [7:0] abys_dumper_tmp166;
            logic [7:0] abys_dumper_tmp168;
    logic abys_dumper_tmp181;
      logic abys_dumper_tmp182;
        logic [7:0] abys_dumper_tmp184;
        logic [7:0] abys_dumper_tmp186;
        logic [7:0] abys_dumper_tmp188;
        logic [7:0] abys_dumper_tmp189;
    abys_dumper_tmp3 = values[1'b0 +: 1];
    abys_dumper_tmp4 = values[1'b1 +: 3];
    abys_dumper_tmp5 = {abys_dumper_tmp3, abys_dumper_tmp4};
    abys_dumper_tmp6 = abys_dumper_tmp5[1'b0];
    abys_dumper_tmp7 = abys_dumper_tmp5[1'b1];
    abys_dumper_tmp9 = abys_dumper_tmp5[2'b10];
    abys_dumper_tmp11 = abys_dumper_tmp5[2'b11];
    abys_dumper_tmp14 = signed_index;
    abys_dumper_tmp16 = (abys_dumper_tmp14 + 4'sb1);
    abys_dumper_tmp18 = ((abys_dumper_tmp16 >> (2'b11)) & {1{1'b1}});
    abys_dumper_tmp20 = ((abys_dumper_tmp16 >> (2'b10)) & {1{1'b1}});
    abys_dumper_tmp21 = ((abys_dumper_tmp16 >> (1'b1)) & {1{1'b1}});
    abys_dumper_tmp22 = ((abys_dumper_tmp16 >> (1'b0)) & {1{1'b1}});
    if (abys_dumper_tmp22) begin
      abys_dumper_tmp23 = 1'b1;
    end else begin
      abys_dumper_tmp23 = 1'b1;
    end
    if (abys_dumper_tmp21) begin
      abys_dumper_tmp24 = 1'b0;
    end else begin
      abys_dumper_tmp24 = abys_dumper_tmp23;
    end
    if (abys_dumper_tmp20) begin
      abys_dumper_tmp25 = 1'b0;
    end else begin
      abys_dumper_tmp25 = abys_dumper_tmp24;
    end
    if (abys_dumper_tmp18) begin
      abys_dumper_tmp26 = 1'b0;
    end else begin
      abys_dumper_tmp26 = abys_dumper_tmp25;
    end
      abys_dumper_tmp28 = ((abys_dumper_tmp16 >> (2'b11)) & {1{1'b1}});
        abys_dumper_tmp30 = update_pair[1'b0];
        abys_dumper_tmp32 = ((abys_dumper_tmp16 >> (2'b10)) & {1{1'b1}});
          abys_dumper_tmp33 = ((abys_dumper_tmp16 >> (1'b1)) & {1{1'b1}});
            abys_dumper_tmp34 = ((abys_dumper_tmp16 >> (1'b0)) & {1{1'b1}});
              abys_dumper_tmp35 = update_pair[1'b1];
      abys_dumper_tmp40 = values[1'b0];
    if (abys_dumper_tmp22) begin
      abys_dumper_tmp42 = 1'b0;
    end else begin
      abys_dumper_tmp42 = 1'b1;
    end
    if (abys_dumper_tmp22) begin
      abys_dumper_tmp43 = 1'b1;
    end else begin
      abys_dumper_tmp43 = 1'b0;
    end
    if (abys_dumper_tmp21) begin
      abys_dumper_tmp44 = abys_dumper_tmp42;
    end else begin
      abys_dumper_tmp44 = abys_dumper_tmp43;
    end
    if (abys_dumper_tmp20) begin
      abys_dumper_tmp45 = 1'b0;
    end else begin
      abys_dumper_tmp45 = abys_dumper_tmp44;
    end
    if (abys_dumper_tmp18) begin
      abys_dumper_tmp46 = 1'b0;
    end else begin
      abys_dumper_tmp46 = abys_dumper_tmp45;
    end
      abys_dumper_tmp52 = values[1'b1];
    if (abys_dumper_tmp22) begin
      abys_dumper_tmp54 = 1'b0;
    end else begin
      abys_dumper_tmp54 = 1'b0;
    end
    if (abys_dumper_tmp21) begin
      abys_dumper_tmp55 = abys_dumper_tmp23;
    end else begin
      abys_dumper_tmp55 = abys_dumper_tmp54;
    end
    if (abys_dumper_tmp20) begin
      abys_dumper_tmp56 = 1'b0;
    end else begin
      abys_dumper_tmp56 = abys_dumper_tmp55;
    end
    if (abys_dumper_tmp18) begin
      abys_dumper_tmp57 = 1'b0;
    end else begin
      abys_dumper_tmp57 = abys_dumper_tmp56;
    end
      abys_dumper_tmp63 = values[2'b10];
    if (abys_dumper_tmp21) begin
      abys_dumper_tmp65 = 1'b0;
    end else begin
      abys_dumper_tmp65 = abys_dumper_tmp42;
    end
    if (abys_dumper_tmp22) begin
      abys_dumper_tmp66 = 1'b0;
    end else begin
      abys_dumper_tmp66 = 1'b0;
    end
    if (abys_dumper_tmp21) begin
      abys_dumper_tmp67 = abys_dumper_tmp43;
    end else begin
      abys_dumper_tmp67 = abys_dumper_tmp66;
    end
    if (abys_dumper_tmp20) begin
      abys_dumper_tmp68 = abys_dumper_tmp65;
    end else begin
      abys_dumper_tmp68 = abys_dumper_tmp67;
    end
    if (abys_dumper_tmp18) begin
      abys_dumper_tmp69 = 1'b0;
    end else begin
      abys_dumper_tmp69 = abys_dumper_tmp68;
    end
      abys_dumper_tmp76 = values[2'b11];
    abys_dumper_tmp80 = index;
    abys_dumper_tmp82 = (abys_dumper_tmp80 + 3'sb1);
    abys_dumper_tmp84 = ((abys_dumper_tmp82 >> (2'b10)) & {1{1'b1}});
    abys_dumper_tmp85 = ((abys_dumper_tmp82 >> (1'b1)) & {1{1'b1}});
    abys_dumper_tmp86 = ((abys_dumper_tmp82 >> (1'b0)) & {1{1'b1}});
    if (abys_dumper_tmp86) begin
      abys_dumper_tmp87 = 1'b1;
    end else begin
      abys_dumper_tmp87 = 1'b1;
    end
    if (abys_dumper_tmp85) begin
      abys_dumper_tmp88 = 1'b0;
    end else begin
      abys_dumper_tmp88 = abys_dumper_tmp87;
    end
    if (abys_dumper_tmp84) begin
      abys_dumper_tmp89 = 1'b0;
    end else begin
      abys_dumper_tmp89 = abys_dumper_tmp88;
    end
      abys_dumper_tmp91 = ((abys_dumper_tmp82 >> (2'b10)) & {1{1'b1}});
        abys_dumper_tmp92 = update_pair[1'b0];
        abys_dumper_tmp93 = ((abys_dumper_tmp82 >> (1'b1)) & {1{1'b1}});
          abys_dumper_tmp94 = ((abys_dumper_tmp82 >> (1'b0)) & {1{1'b1}});
            abys_dumper_tmp95 = update_pair[1'b1];
      abys_dumper_tmp99 = values[1'b0];
    if (abys_dumper_tmp86) begin
      abys_dumper_tmp101 = 1'b0;
    end else begin
      abys_dumper_tmp101 = 1'b1;
    end
    if (abys_dumper_tmp86) begin
      abys_dumper_tmp102 = 1'b1;
    end else begin
      abys_dumper_tmp102 = 1'b0;
    end
    if (abys_dumper_tmp85) begin
      abys_dumper_tmp103 = abys_dumper_tmp101;
    end else begin
      abys_dumper_tmp103 = abys_dumper_tmp102;
    end
    if (abys_dumper_tmp84) begin
      abys_dumper_tmp104 = 1'b0;
    end else begin
      abys_dumper_tmp104 = abys_dumper_tmp103;
    end
      abys_dumper_tmp109 = values[1'b1];
    if (abys_dumper_tmp86) begin
      abys_dumper_tmp111 = 1'b0;
    end else begin
      abys_dumper_tmp111 = 1'b0;
    end
    if (abys_dumper_tmp85) begin
      abys_dumper_tmp112 = abys_dumper_tmp87;
    end else begin
      abys_dumper_tmp112 = abys_dumper_tmp111;
    end
    if (abys_dumper_tmp84) begin
      abys_dumper_tmp113 = 1'b0;
    end else begin
      abys_dumper_tmp113 = abys_dumper_tmp112;
    end
      abys_dumper_tmp118 = values[2'b10];
    if (abys_dumper_tmp85) begin
      abys_dumper_tmp120 = 1'b0;
    end else begin
      abys_dumper_tmp120 = abys_dumper_tmp101;
    end
    if (abys_dumper_tmp86) begin
      abys_dumper_tmp121 = 1'b0;
    end else begin
      abys_dumper_tmp121 = 1'b0;
    end
    if (abys_dumper_tmp85) begin
      abys_dumper_tmp122 = abys_dumper_tmp102;
    end else begin
      abys_dumper_tmp122 = abys_dumper_tmp121;
    end
    if (abys_dumper_tmp84) begin
      abys_dumper_tmp123 = abys_dumper_tmp120;
    end else begin
      abys_dumper_tmp123 = abys_dumper_tmp122;
    end
      abys_dumper_tmp129 = values[2'b11];
    abys_dumper_tmp132 = (index == 1'b0);
      abys_dumper_tmp134 = values[1'b0];
    abys_dumper_tmp136 = (index == 1'b1);
      abys_dumper_tmp137 = values[1'b1];
    abys_dumper_tmp140 = (index == 2'b10);
      abys_dumper_tmp142 = values[2'b10];
    abys_dumper_tmp145 = (index == 2'b11);
      abys_dumper_tmp147 = values[2'b11];
    abys_dumper_tmp150 = index;
    abys_dumper_tmp152 = (abys_dumper_tmp150 + 4'sb1);
    abys_dumper_tmp154 = ((abys_dumper_tmp152 >> (2'b11)) & {1{1'b1}});
      abys_dumper_tmp157 = ((abys_dumper_tmp152 >> (2'b10)) & {1{1'b1}});
        abys_dumper_tmp158 = ((abys_dumper_tmp152 >> (1'b1)) & {1{1'b1}});
          abys_dumper_tmp159 = ((abys_dumper_tmp152 >> (1'b0)) & {1{1'b1}});
            abys_dumper_tmp161 = values[2'b11];
            abys_dumper_tmp165 = values[2'b10];
            abys_dumper_tmp166 = values[1'b1];
            abys_dumper_tmp168 = values[1'b0];
    abys_dumper_tmp181 = index[1'b1];
      abys_dumper_tmp182 = index[1'b0];
        abys_dumper_tmp184 = values[2'b11];
        abys_dumper_tmp186 = values[2'b10];
        abys_dumper_tmp188 = values[1'b1];
        abys_dumper_tmp189 = values[1'b0];
    concatenated[0] = abys_dumper_tmp6;
    concatenated[1] = abys_dumper_tmp7;
    concatenated[2] = abys_dumper_tmp9;
    concatenated[3] = abys_dumper_tmp11;
    if (abys_dumper_tmp26) begin
      if (abys_dumper_tmp28) begin
        updated_negative_range[0] = abys_dumper_tmp30;
      end else begin
        if (abys_dumper_tmp32) begin
          updated_negative_range[0] = abys_dumper_tmp30;
        end else begin
          if (abys_dumper_tmp33) begin
            updated_negative_range[0] = abys_dumper_tmp30;
          end else begin
            if (abys_dumper_tmp34) begin
              updated_negative_range[0] = abys_dumper_tmp30;
            end else begin
              updated_negative_range[0] = abys_dumper_tmp35;
            end
          end
        end
      end
    end else begin
      updated_negative_range[0] = abys_dumper_tmp40;
    end
    if (abys_dumper_tmp46) begin
      if (abys_dumper_tmp28) begin
        updated_negative_range[1] = abys_dumper_tmp30;
      end else begin
        if (abys_dumper_tmp32) begin
          updated_negative_range[1] = abys_dumper_tmp30;
        end else begin
          if (abys_dumper_tmp33) begin
            if (abys_dumper_tmp34) begin
              updated_negative_range[1] = abys_dumper_tmp30;
            end else begin
              updated_negative_range[1] = abys_dumper_tmp30;
            end
          end else begin
            if (abys_dumper_tmp34) begin
              updated_negative_range[1] = abys_dumper_tmp35;
            end else begin
              updated_negative_range[1] = abys_dumper_tmp30;
            end
          end
        end
      end
    end else begin
      updated_negative_range[1] = abys_dumper_tmp52;
    end
    if (abys_dumper_tmp57) begin
      if (abys_dumper_tmp28) begin
        updated_negative_range[2] = abys_dumper_tmp30;
      end else begin
        if (abys_dumper_tmp32) begin
          updated_negative_range[2] = abys_dumper_tmp30;
        end else begin
          if (abys_dumper_tmp33) begin
            if (abys_dumper_tmp34) begin
              updated_negative_range[2] = abys_dumper_tmp30;
            end else begin
              updated_negative_range[2] = abys_dumper_tmp35;
            end
          end else begin
            if (abys_dumper_tmp34) begin
              updated_negative_range[2] = abys_dumper_tmp30;
            end else begin
              updated_negative_range[2] = abys_dumper_tmp30;
            end
          end
        end
      end
    end else begin
      updated_negative_range[2] = abys_dumper_tmp63;
    end
    if (abys_dumper_tmp69) begin
      if (abys_dumper_tmp28) begin
        updated_negative_range[3] = abys_dumper_tmp30;
      end else begin
        if (abys_dumper_tmp32) begin
          if (abys_dumper_tmp33) begin
            updated_negative_range[3] = abys_dumper_tmp30;
          end else begin
            if (abys_dumper_tmp34) begin
              updated_negative_range[3] = abys_dumper_tmp30;
            end else begin
              updated_negative_range[3] = abys_dumper_tmp30;
            end
          end
        end else begin
          if (abys_dumper_tmp33) begin
            if (abys_dumper_tmp34) begin
              updated_negative_range[3] = abys_dumper_tmp35;
            end else begin
              updated_negative_range[3] = abys_dumper_tmp30;
            end
          end else begin
            if (abys_dumper_tmp34) begin
              updated_negative_range[3] = abys_dumper_tmp30;
            end else begin
              updated_negative_range[3] = abys_dumper_tmp30;
            end
          end
        end
      end
    end else begin
      updated_negative_range[3] = abys_dumper_tmp76;
    end
    if (abys_dumper_tmp89) begin
      if (abys_dumper_tmp91) begin
        updated_range[0] = abys_dumper_tmp92;
      end else begin
        if (abys_dumper_tmp93) begin
          updated_range[0] = abys_dumper_tmp92;
        end else begin
          if (abys_dumper_tmp94) begin
            updated_range[0] = abys_dumper_tmp92;
          end else begin
            updated_range[0] = abys_dumper_tmp95;
          end
        end
      end
    end else begin
      updated_range[0] = abys_dumper_tmp99;
    end
    if (abys_dumper_tmp104) begin
      if (abys_dumper_tmp91) begin
        updated_range[1] = abys_dumper_tmp92;
      end else begin
        if (abys_dumper_tmp93) begin
          if (abys_dumper_tmp94) begin
            updated_range[1] = abys_dumper_tmp92;
          end else begin
            updated_range[1] = abys_dumper_tmp92;
          end
        end else begin
          if (abys_dumper_tmp94) begin
            updated_range[1] = abys_dumper_tmp95;
          end else begin
            updated_range[1] = abys_dumper_tmp92;
          end
        end
      end
    end else begin
      updated_range[1] = abys_dumper_tmp109;
    end
    if (abys_dumper_tmp113) begin
      if (abys_dumper_tmp91) begin
        updated_range[2] = abys_dumper_tmp92;
      end else begin
        if (abys_dumper_tmp93) begin
          if (abys_dumper_tmp94) begin
            updated_range[2] = abys_dumper_tmp92;
          end else begin
            updated_range[2] = abys_dumper_tmp95;
          end
        end else begin
          if (abys_dumper_tmp94) begin
            updated_range[2] = abys_dumper_tmp92;
          end else begin
            updated_range[2] = abys_dumper_tmp92;
          end
        end
      end
    end else begin
      updated_range[2] = abys_dumper_tmp118;
    end
    if (abys_dumper_tmp123) begin
      if (abys_dumper_tmp91) begin
        if (abys_dumper_tmp93) begin
          updated_range[3] = abys_dumper_tmp92;
        end else begin
          if (abys_dumper_tmp94) begin
            updated_range[3] = abys_dumper_tmp92;
          end else begin
            updated_range[3] = abys_dumper_tmp92;
          end
        end
      end else begin
        if (abys_dumper_tmp93) begin
          if (abys_dumper_tmp94) begin
            updated_range[3] = abys_dumper_tmp95;
          end else begin
            updated_range[3] = abys_dumper_tmp92;
          end
        end else begin
          if (abys_dumper_tmp94) begin
            updated_range[3] = abys_dumper_tmp92;
          end else begin
            updated_range[3] = abys_dumper_tmp92;
          end
        end
      end
    end else begin
      updated_range[3] = abys_dumper_tmp129;
    end
    if (abys_dumper_tmp132) begin
      updated[0] = update;
    end else begin
      updated[0] = abys_dumper_tmp134;
    end
    if (abys_dumper_tmp136) begin
      updated[1] = update;
    end else begin
      updated[1] = abys_dumper_tmp137;
    end
    if (abys_dumper_tmp140) begin
      updated[2] = update;
    end else begin
      updated[2] = abys_dumper_tmp142;
    end
    if (abys_dumper_tmp145) begin
      updated[3] = update;
    end else begin
      updated[3] = abys_dumper_tmp147;
    end
    if (abys_dumper_tmp154) begin
      selected_range[0] = 8'bx;
    end else begin
      if (abys_dumper_tmp157) begin
        if (abys_dumper_tmp158) begin
          selected_range[0] = 8'bx;
        end else begin
          if (abys_dumper_tmp159) begin
            selected_range[0] = 8'bx;
          end else begin
            selected_range[0] = abys_dumper_tmp161;
          end
        end
      end else begin
        if (abys_dumper_tmp158) begin
          if (abys_dumper_tmp159) begin
            selected_range[0] = abys_dumper_tmp165;
          end else begin
            selected_range[0] = abys_dumper_tmp166;
          end
        end else begin
          if (abys_dumper_tmp159) begin
            selected_range[0] = abys_dumper_tmp168;
          end else begin
            selected_range[0] = 8'bx;
          end
        end
      end
    end
    if (abys_dumper_tmp154) begin
      selected_range[1] = 8'bx;
    end else begin
      if (abys_dumper_tmp157) begin
        if (abys_dumper_tmp158) begin
          selected_range[1] = 8'bx;
        end else begin
          if (abys_dumper_tmp159) begin
            selected_range[1] = 8'bx;
          end else begin
            selected_range[1] = 8'bx;
          end
        end
      end else begin
        if (abys_dumper_tmp158) begin
          if (abys_dumper_tmp159) begin
            selected_range[1] = abys_dumper_tmp161;
          end else begin
            selected_range[1] = abys_dumper_tmp165;
          end
        end else begin
          if (abys_dumper_tmp159) begin
            selected_range[1] = abys_dumper_tmp166;
          end else begin
            selected_range[1] = abys_dumper_tmp168;
          end
        end
      end
    end
    if (abys_dumper_tmp181) begin
      if (abys_dumper_tmp182) begin
        selected = abys_dumper_tmp184;
      end else begin
        selected = abys_dumper_tmp186;
      end
    end else begin
      if (abys_dumper_tmp182) begin
        selected = abys_dumper_tmp188;
      end else begin
        selected = abys_dumper_tmp189;
      end
    end
  end
endmodule
