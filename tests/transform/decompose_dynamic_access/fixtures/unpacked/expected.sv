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
    logic abys_dumper_tmp17;
    logic signed [3:0] abys_dumper_tmp14;
    logic signed [3:0] abys_dumper_tmp16;
    logic abys_dumper_tmp18;
    logic abys_dumper_tmp19;
    logic abys_dumper_tmp20;
    logic abys_dumper_tmp21;
    logic abys_dumper_tmp22;
    logic abys_dumper_tmp23;
    logic abys_dumper_tmp24;
      logic abys_dumper_tmp25;
        logic [7:0] abys_dumper_tmp27;
        logic abys_dumper_tmp28;
          logic abys_dumper_tmp29;
            logic abys_dumper_tmp30;
              logic [7:0] abys_dumper_tmp31;
      logic [7:0] abys_dumper_tmp36;
    logic abys_dumper_tmp38;
    logic abys_dumper_tmp39;
    logic abys_dumper_tmp40;
    logic abys_dumper_tmp41;
    logic abys_dumper_tmp42;
      logic [7:0] abys_dumper_tmp48;
    logic abys_dumper_tmp50;
    logic abys_dumper_tmp51;
    logic abys_dumper_tmp52;
    logic abys_dumper_tmp53;
      logic [7:0] abys_dumper_tmp59;
    logic abys_dumper_tmp61;
    logic abys_dumper_tmp62;
    logic abys_dumper_tmp63;
    logic abys_dumper_tmp64;
    logic abys_dumper_tmp65;
      logic [7:0] abys_dumper_tmp72;
    logic abys_dumper_tmp79;
    logic signed [2:0] abys_dumper_tmp76;
    logic signed [2:0] abys_dumper_tmp78;
    logic abys_dumper_tmp80;
    logic abys_dumper_tmp81;
    logic abys_dumper_tmp82;
    logic abys_dumper_tmp83;
    logic abys_dumper_tmp84;
      logic abys_dumper_tmp85;
        logic [7:0] abys_dumper_tmp86;
        logic abys_dumper_tmp87;
          logic abys_dumper_tmp88;
            logic [7:0] abys_dumper_tmp89;
      logic [7:0] abys_dumper_tmp93;
    logic abys_dumper_tmp95;
    logic abys_dumper_tmp96;
    logic abys_dumper_tmp97;
    logic abys_dumper_tmp98;
      logic [7:0] abys_dumper_tmp103;
    logic abys_dumper_tmp105;
    logic abys_dumper_tmp106;
    logic abys_dumper_tmp107;
      logic [7:0] abys_dumper_tmp112;
    logic abys_dumper_tmp114;
    logic abys_dumper_tmp115;
    logic abys_dumper_tmp116;
    logic abys_dumper_tmp117;
      logic [7:0] abys_dumper_tmp123;
    logic abys_dumper_tmp126;
      logic [7:0] abys_dumper_tmp128;
    logic abys_dumper_tmp130;
      logic [7:0] abys_dumper_tmp131;
    logic abys_dumper_tmp134;
      logic [7:0] abys_dumper_tmp136;
    logic abys_dumper_tmp139;
      logic [7:0] abys_dumper_tmp141;
    logic abys_dumper_tmp147;
    logic signed [2:0] abys_dumper_tmp144;
    logic signed [3:0] abys_dumper_tmp146;
      logic abys_dumper_tmp149;
        logic abys_dumper_tmp150;
          logic abys_dumper_tmp151;
            logic [7:0] abys_dumper_tmp153;
            logic [7:0] abys_dumper_tmp157;
            logic [7:0] abys_dumper_tmp158;
            logic [7:0] abys_dumper_tmp160;
    logic abys_dumper_tmp173;
      logic abys_dumper_tmp174;
        logic [7:0] abys_dumper_tmp176;
        logic [7:0] abys_dumper_tmp178;
        logic [7:0] abys_dumper_tmp180;
        logic [7:0] abys_dumper_tmp181;
    abys_dumper_tmp3 = values[1'b0 +: 1];
    abys_dumper_tmp4 = values[1'b1 +: 3];
    abys_dumper_tmp5 = {abys_dumper_tmp3, abys_dumper_tmp4};
    abys_dumper_tmp6 = abys_dumper_tmp5[1'b0];
    abys_dumper_tmp7 = abys_dumper_tmp5[1'b1];
    abys_dumper_tmp9 = abys_dumper_tmp5[2'b10];
    abys_dumper_tmp11 = abys_dumper_tmp5[2'b11];
    abys_dumper_tmp14 = signed_index;
    abys_dumper_tmp16 = (abys_dumper_tmp14 + 4'sb1);
    abys_dumper_tmp17 = ((abys_dumper_tmp16 >> (2'b11)) & {1{1'b1}});
    abys_dumper_tmp18 = ((abys_dumper_tmp16 >> (2'b10)) & {1{1'b1}});
    abys_dumper_tmp19 = ((abys_dumper_tmp16 >> (1'b1)) & {1{1'b1}});
    abys_dumper_tmp20 = ((abys_dumper_tmp16 >> (1'b0)) & {1{1'b1}});
    if (abys_dumper_tmp20) begin
      abys_dumper_tmp21 = 1'b1;
    end else begin
      abys_dumper_tmp21 = 1'b1;
    end
    if (abys_dumper_tmp19) begin
      abys_dumper_tmp22 = 1'b0;
    end else begin
      abys_dumper_tmp22 = abys_dumper_tmp21;
    end
    if (abys_dumper_tmp18) begin
      abys_dumper_tmp23 = 1'b0;
    end else begin
      abys_dumper_tmp23 = abys_dumper_tmp22;
    end
    if (abys_dumper_tmp17) begin
      abys_dumper_tmp24 = 1'b0;
    end else begin
      abys_dumper_tmp24 = abys_dumper_tmp23;
    end
      abys_dumper_tmp25 = ((abys_dumper_tmp16 >> (2'b11)) & {1{1'b1}});
        abys_dumper_tmp27 = update_pair[1'b0];
        abys_dumper_tmp28 = ((abys_dumper_tmp16 >> (2'b10)) & {1{1'b1}});
          abys_dumper_tmp29 = ((abys_dumper_tmp16 >> (1'b1)) & {1{1'b1}});
            abys_dumper_tmp30 = ((abys_dumper_tmp16 >> (1'b0)) & {1{1'b1}});
              abys_dumper_tmp31 = update_pair[1'b1];
      abys_dumper_tmp36 = values[1'b0];
    if (abys_dumper_tmp20) begin
      abys_dumper_tmp38 = 1'b0;
    end else begin
      abys_dumper_tmp38 = 1'b1;
    end
    if (abys_dumper_tmp20) begin
      abys_dumper_tmp39 = 1'b1;
    end else begin
      abys_dumper_tmp39 = 1'b0;
    end
    if (abys_dumper_tmp19) begin
      abys_dumper_tmp40 = abys_dumper_tmp38;
    end else begin
      abys_dumper_tmp40 = abys_dumper_tmp39;
    end
    if (abys_dumper_tmp18) begin
      abys_dumper_tmp41 = 1'b0;
    end else begin
      abys_dumper_tmp41 = abys_dumper_tmp40;
    end
    if (abys_dumper_tmp17) begin
      abys_dumper_tmp42 = 1'b0;
    end else begin
      abys_dumper_tmp42 = abys_dumper_tmp41;
    end
      abys_dumper_tmp48 = values[1'b1];
    if (abys_dumper_tmp20) begin
      abys_dumper_tmp50 = 1'b0;
    end else begin
      abys_dumper_tmp50 = 1'b0;
    end
    if (abys_dumper_tmp19) begin
      abys_dumper_tmp51 = abys_dumper_tmp21;
    end else begin
      abys_dumper_tmp51 = abys_dumper_tmp50;
    end
    if (abys_dumper_tmp18) begin
      abys_dumper_tmp52 = 1'b0;
    end else begin
      abys_dumper_tmp52 = abys_dumper_tmp51;
    end
    if (abys_dumper_tmp17) begin
      abys_dumper_tmp53 = 1'b0;
    end else begin
      abys_dumper_tmp53 = abys_dumper_tmp52;
    end
      abys_dumper_tmp59 = values[2'b10];
    if (abys_dumper_tmp19) begin
      abys_dumper_tmp61 = 1'b0;
    end else begin
      abys_dumper_tmp61 = abys_dumper_tmp38;
    end
    if (abys_dumper_tmp20) begin
      abys_dumper_tmp62 = 1'b0;
    end else begin
      abys_dumper_tmp62 = 1'b0;
    end
    if (abys_dumper_tmp19) begin
      abys_dumper_tmp63 = abys_dumper_tmp39;
    end else begin
      abys_dumper_tmp63 = abys_dumper_tmp62;
    end
    if (abys_dumper_tmp18) begin
      abys_dumper_tmp64 = abys_dumper_tmp61;
    end else begin
      abys_dumper_tmp64 = abys_dumper_tmp63;
    end
    if (abys_dumper_tmp17) begin
      abys_dumper_tmp65 = 1'b0;
    end else begin
      abys_dumper_tmp65 = abys_dumper_tmp64;
    end
      abys_dumper_tmp72 = values[2'b11];
    abys_dumper_tmp76 = index;
    abys_dumper_tmp78 = (abys_dumper_tmp76 + 3'sb1);
    abys_dumper_tmp79 = ((abys_dumper_tmp78 >> (2'b10)) & {1{1'b1}});
    abys_dumper_tmp80 = ((abys_dumper_tmp78 >> (1'b1)) & {1{1'b1}});
    abys_dumper_tmp81 = ((abys_dumper_tmp78 >> (1'b0)) & {1{1'b1}});
    if (abys_dumper_tmp81) begin
      abys_dumper_tmp82 = 1'b1;
    end else begin
      abys_dumper_tmp82 = 1'b1;
    end
    if (abys_dumper_tmp80) begin
      abys_dumper_tmp83 = 1'b0;
    end else begin
      abys_dumper_tmp83 = abys_dumper_tmp82;
    end
    if (abys_dumper_tmp79) begin
      abys_dumper_tmp84 = 1'b0;
    end else begin
      abys_dumper_tmp84 = abys_dumper_tmp83;
    end
      abys_dumper_tmp85 = ((abys_dumper_tmp78 >> (2'b10)) & {1{1'b1}});
        abys_dumper_tmp86 = update_pair[1'b0];
        abys_dumper_tmp87 = ((abys_dumper_tmp78 >> (1'b1)) & {1{1'b1}});
          abys_dumper_tmp88 = ((abys_dumper_tmp78 >> (1'b0)) & {1{1'b1}});
            abys_dumper_tmp89 = update_pair[1'b1];
      abys_dumper_tmp93 = values[1'b0];
    if (abys_dumper_tmp81) begin
      abys_dumper_tmp95 = 1'b0;
    end else begin
      abys_dumper_tmp95 = 1'b1;
    end
    if (abys_dumper_tmp81) begin
      abys_dumper_tmp96 = 1'b1;
    end else begin
      abys_dumper_tmp96 = 1'b0;
    end
    if (abys_dumper_tmp80) begin
      abys_dumper_tmp97 = abys_dumper_tmp95;
    end else begin
      abys_dumper_tmp97 = abys_dumper_tmp96;
    end
    if (abys_dumper_tmp79) begin
      abys_dumper_tmp98 = 1'b0;
    end else begin
      abys_dumper_tmp98 = abys_dumper_tmp97;
    end
      abys_dumper_tmp103 = values[1'b1];
    if (abys_dumper_tmp81) begin
      abys_dumper_tmp105 = 1'b0;
    end else begin
      abys_dumper_tmp105 = 1'b0;
    end
    if (abys_dumper_tmp80) begin
      abys_dumper_tmp106 = abys_dumper_tmp82;
    end else begin
      abys_dumper_tmp106 = abys_dumper_tmp105;
    end
    if (abys_dumper_tmp79) begin
      abys_dumper_tmp107 = 1'b0;
    end else begin
      abys_dumper_tmp107 = abys_dumper_tmp106;
    end
      abys_dumper_tmp112 = values[2'b10];
    if (abys_dumper_tmp80) begin
      abys_dumper_tmp114 = 1'b0;
    end else begin
      abys_dumper_tmp114 = abys_dumper_tmp95;
    end
    if (abys_dumper_tmp81) begin
      abys_dumper_tmp115 = 1'b0;
    end else begin
      abys_dumper_tmp115 = 1'b0;
    end
    if (abys_dumper_tmp80) begin
      abys_dumper_tmp116 = abys_dumper_tmp96;
    end else begin
      abys_dumper_tmp116 = abys_dumper_tmp115;
    end
    if (abys_dumper_tmp79) begin
      abys_dumper_tmp117 = abys_dumper_tmp114;
    end else begin
      abys_dumper_tmp117 = abys_dumper_tmp116;
    end
      abys_dumper_tmp123 = values[2'b11];
    abys_dumper_tmp126 = (index == 1'b0);
      abys_dumper_tmp128 = values[1'b0];
    abys_dumper_tmp130 = (index == 1'b1);
      abys_dumper_tmp131 = values[1'b1];
    abys_dumper_tmp134 = (index == 2'b10);
      abys_dumper_tmp136 = values[2'b10];
    abys_dumper_tmp139 = (index == 2'b11);
      abys_dumper_tmp141 = values[2'b11];
    abys_dumper_tmp144 = index;
    abys_dumper_tmp146 = (abys_dumper_tmp144 + 4'sb1);
    abys_dumper_tmp147 = ((abys_dumper_tmp146 >> (2'b11)) & {1{1'b1}});
      abys_dumper_tmp149 = ((abys_dumper_tmp146 >> (2'b10)) & {1{1'b1}});
        abys_dumper_tmp150 = ((abys_dumper_tmp146 >> (1'b1)) & {1{1'b1}});
          abys_dumper_tmp151 = ((abys_dumper_tmp146 >> (1'b0)) & {1{1'b1}});
            abys_dumper_tmp153 = values[2'b11];
            abys_dumper_tmp157 = values[2'b10];
            abys_dumper_tmp158 = values[1'b1];
            abys_dumper_tmp160 = values[1'b0];
    abys_dumper_tmp173 = index[1'b1];
      abys_dumper_tmp174 = index[1'b0];
        abys_dumper_tmp176 = values[2'b11];
        abys_dumper_tmp178 = values[2'b10];
        abys_dumper_tmp180 = values[1'b1];
        abys_dumper_tmp181 = values[1'b0];
    concatenated[0] = abys_dumper_tmp6;
    concatenated[1] = abys_dumper_tmp7;
    concatenated[2] = abys_dumper_tmp9;
    concatenated[3] = abys_dumper_tmp11;
    if (abys_dumper_tmp24) begin
      if (abys_dumper_tmp25) begin
        updated_negative_range[0] = abys_dumper_tmp27;
      end else begin
        if (abys_dumper_tmp28) begin
          updated_negative_range[0] = abys_dumper_tmp27;
        end else begin
          if (abys_dumper_tmp29) begin
            updated_negative_range[0] = abys_dumper_tmp27;
          end else begin
            if (abys_dumper_tmp30) begin
              updated_negative_range[0] = abys_dumper_tmp27;
            end else begin
              updated_negative_range[0] = abys_dumper_tmp31;
            end
          end
        end
      end
    end else begin
      updated_negative_range[0] = abys_dumper_tmp36;
    end
    if (abys_dumper_tmp42) begin
      if (abys_dumper_tmp25) begin
        updated_negative_range[1] = abys_dumper_tmp27;
      end else begin
        if (abys_dumper_tmp28) begin
          updated_negative_range[1] = abys_dumper_tmp27;
        end else begin
          if (abys_dumper_tmp29) begin
            if (abys_dumper_tmp30) begin
              updated_negative_range[1] = abys_dumper_tmp27;
            end else begin
              updated_negative_range[1] = abys_dumper_tmp27;
            end
          end else begin
            if (abys_dumper_tmp30) begin
              updated_negative_range[1] = abys_dumper_tmp31;
            end else begin
              updated_negative_range[1] = abys_dumper_tmp27;
            end
          end
        end
      end
    end else begin
      updated_negative_range[1] = abys_dumper_tmp48;
    end
    if (abys_dumper_tmp53) begin
      if (abys_dumper_tmp25) begin
        updated_negative_range[2] = abys_dumper_tmp27;
      end else begin
        if (abys_dumper_tmp28) begin
          updated_negative_range[2] = abys_dumper_tmp27;
        end else begin
          if (abys_dumper_tmp29) begin
            if (abys_dumper_tmp30) begin
              updated_negative_range[2] = abys_dumper_tmp27;
            end else begin
              updated_negative_range[2] = abys_dumper_tmp31;
            end
          end else begin
            if (abys_dumper_tmp30) begin
              updated_negative_range[2] = abys_dumper_tmp27;
            end else begin
              updated_negative_range[2] = abys_dumper_tmp27;
            end
          end
        end
      end
    end else begin
      updated_negative_range[2] = abys_dumper_tmp59;
    end
    if (abys_dumper_tmp65) begin
      if (abys_dumper_tmp25) begin
        updated_negative_range[3] = abys_dumper_tmp27;
      end else begin
        if (abys_dumper_tmp28) begin
          if (abys_dumper_tmp29) begin
            updated_negative_range[3] = abys_dumper_tmp27;
          end else begin
            if (abys_dumper_tmp30) begin
              updated_negative_range[3] = abys_dumper_tmp27;
            end else begin
              updated_negative_range[3] = abys_dumper_tmp27;
            end
          end
        end else begin
          if (abys_dumper_tmp29) begin
            if (abys_dumper_tmp30) begin
              updated_negative_range[3] = abys_dumper_tmp31;
            end else begin
              updated_negative_range[3] = abys_dumper_tmp27;
            end
          end else begin
            if (abys_dumper_tmp30) begin
              updated_negative_range[3] = abys_dumper_tmp27;
            end else begin
              updated_negative_range[3] = abys_dumper_tmp27;
            end
          end
        end
      end
    end else begin
      updated_negative_range[3] = abys_dumper_tmp72;
    end
    if (abys_dumper_tmp84) begin
      if (abys_dumper_tmp85) begin
        updated_range[0] = abys_dumper_tmp86;
      end else begin
        if (abys_dumper_tmp87) begin
          updated_range[0] = abys_dumper_tmp86;
        end else begin
          if (abys_dumper_tmp88) begin
            updated_range[0] = abys_dumper_tmp86;
          end else begin
            updated_range[0] = abys_dumper_tmp89;
          end
        end
      end
    end else begin
      updated_range[0] = abys_dumper_tmp93;
    end
    if (abys_dumper_tmp98) begin
      if (abys_dumper_tmp85) begin
        updated_range[1] = abys_dumper_tmp86;
      end else begin
        if (abys_dumper_tmp87) begin
          if (abys_dumper_tmp88) begin
            updated_range[1] = abys_dumper_tmp86;
          end else begin
            updated_range[1] = abys_dumper_tmp86;
          end
        end else begin
          if (abys_dumper_tmp88) begin
            updated_range[1] = abys_dumper_tmp89;
          end else begin
            updated_range[1] = abys_dumper_tmp86;
          end
        end
      end
    end else begin
      updated_range[1] = abys_dumper_tmp103;
    end
    if (abys_dumper_tmp107) begin
      if (abys_dumper_tmp85) begin
        updated_range[2] = abys_dumper_tmp86;
      end else begin
        if (abys_dumper_tmp87) begin
          if (abys_dumper_tmp88) begin
            updated_range[2] = abys_dumper_tmp86;
          end else begin
            updated_range[2] = abys_dumper_tmp89;
          end
        end else begin
          if (abys_dumper_tmp88) begin
            updated_range[2] = abys_dumper_tmp86;
          end else begin
            updated_range[2] = abys_dumper_tmp86;
          end
        end
      end
    end else begin
      updated_range[2] = abys_dumper_tmp112;
    end
    if (abys_dumper_tmp117) begin
      if (abys_dumper_tmp85) begin
        if (abys_dumper_tmp87) begin
          updated_range[3] = abys_dumper_tmp86;
        end else begin
          if (abys_dumper_tmp88) begin
            updated_range[3] = abys_dumper_tmp86;
          end else begin
            updated_range[3] = abys_dumper_tmp86;
          end
        end
      end else begin
        if (abys_dumper_tmp87) begin
          if (abys_dumper_tmp88) begin
            updated_range[3] = abys_dumper_tmp89;
          end else begin
            updated_range[3] = abys_dumper_tmp86;
          end
        end else begin
          if (abys_dumper_tmp88) begin
            updated_range[3] = abys_dumper_tmp86;
          end else begin
            updated_range[3] = abys_dumper_tmp86;
          end
        end
      end
    end else begin
      updated_range[3] = abys_dumper_tmp123;
    end
    if (abys_dumper_tmp126) begin
      updated[0] = update;
    end else begin
      updated[0] = abys_dumper_tmp128;
    end
    if (abys_dumper_tmp130) begin
      updated[1] = update;
    end else begin
      updated[1] = abys_dumper_tmp131;
    end
    if (abys_dumper_tmp134) begin
      updated[2] = update;
    end else begin
      updated[2] = abys_dumper_tmp136;
    end
    if (abys_dumper_tmp139) begin
      updated[3] = update;
    end else begin
      updated[3] = abys_dumper_tmp141;
    end
    if (abys_dumper_tmp147) begin
      selected_range[0] = 8'bx;
    end else begin
      if (abys_dumper_tmp149) begin
        if (abys_dumper_tmp150) begin
          selected_range[0] = 8'bx;
        end else begin
          if (abys_dumper_tmp151) begin
            selected_range[0] = 8'bx;
          end else begin
            selected_range[0] = abys_dumper_tmp153;
          end
        end
      end else begin
        if (abys_dumper_tmp150) begin
          if (abys_dumper_tmp151) begin
            selected_range[0] = abys_dumper_tmp157;
          end else begin
            selected_range[0] = abys_dumper_tmp158;
          end
        end else begin
          if (abys_dumper_tmp151) begin
            selected_range[0] = abys_dumper_tmp160;
          end else begin
            selected_range[0] = 8'bx;
          end
        end
      end
    end
    if (abys_dumper_tmp147) begin
      selected_range[1] = 8'bx;
    end else begin
      if (abys_dumper_tmp149) begin
        if (abys_dumper_tmp150) begin
          selected_range[1] = 8'bx;
        end else begin
          if (abys_dumper_tmp151) begin
            selected_range[1] = 8'bx;
          end else begin
            selected_range[1] = 8'bx;
          end
        end
      end else begin
        if (abys_dumper_tmp150) begin
          if (abys_dumper_tmp151) begin
            selected_range[1] = abys_dumper_tmp153;
          end else begin
            selected_range[1] = abys_dumper_tmp157;
          end
        end else begin
          if (abys_dumper_tmp151) begin
            selected_range[1] = abys_dumper_tmp158;
          end else begin
            selected_range[1] = abys_dumper_tmp160;
          end
        end
      end
    end
    if (abys_dumper_tmp173) begin
      if (abys_dumper_tmp174) begin
        selected = abys_dumper_tmp176;
      end else begin
        selected = abys_dumper_tmp178;
      end
    end else begin
      if (abys_dumper_tmp174) begin
        selected = abys_dumper_tmp180;
      end else begin
        selected = abys_dumper_tmp181;
      end
    end
  end
endmodule
