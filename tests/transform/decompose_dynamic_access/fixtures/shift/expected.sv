module top (
  input [12:0] data,
  input signed [12:0] signed_data,
  input [3:0] amount,
  output  logic [12:0] left,
  output  logic [12:0] right,
  output  logic signed [12:0] arithmetic_right);



  always @(*)   begin
    logic abys_dumper_tmp3;
    logic abys_dumper_tmp4;
    logic abys_dumper_tmp5;
    logic abys_dumper_tmp6;
    logic abys_dumper_tmp8;
    logic abys_dumper_tmp9;
    logic abys_dumper_tmp10;
    logic abys_dumper_tmp11;
    logic abys_dumper_tmp12;
    logic abys_dumper_tmp13;
    logic abys_dumper_tmp14;
    logic abys_dumper_tmp15;
    logic abys_dumper_tmp16;
    logic abys_dumper_tmp17;
    logic abys_dumper_tmp18;
    logic abys_dumper_tmp19;
    logic abys_dumper_tmp20;
    logic abys_dumper_tmp21;
    logic abys_dumper_tmp22;
    logic abys_dumper_tmp23;
    logic abys_dumper_tmp24;
    logic abys_dumper_tmp25;
    logic abys_dumper_tmp26;
    logic abys_dumper_tmp27;
    logic abys_dumper_tmp28;
    logic abys_dumper_tmp29;
    logic abys_dumper_tmp30;
    logic abys_dumper_tmp31;
    logic abys_dumper_tmp32;
    logic abys_dumper_tmp33;
    logic abys_dumper_tmp34;
    logic abys_dumper_tmp35;
    logic abys_dumper_tmp36;
    logic abys_dumper_tmp37;
    logic abys_dumper_tmp38;
    logic abys_dumper_tmp39;
    logic abys_dumper_tmp40;
    logic abys_dumper_tmp41;
    logic abys_dumper_tmp42;
    logic abys_dumper_tmp43;
    logic abys_dumper_tmp44;
    logic abys_dumper_tmp45;
    logic abys_dumper_tmp46;
    logic abys_dumper_tmp47;
    logic abys_dumper_tmp48;
    logic abys_dumper_tmp49;
    logic abys_dumper_tmp50;
    logic abys_dumper_tmp51;
    logic abys_dumper_tmp52;
    logic abys_dumper_tmp53;
    logic abys_dumper_tmp54;
    logic abys_dumper_tmp55;
    logic abys_dumper_tmp56;
    logic abys_dumper_tmp57;
    logic abys_dumper_tmp58;
    logic abys_dumper_tmp59;
    logic abys_dumper_tmp60;
    logic abys_dumper_tmp61;
    logic abys_dumper_tmp62;
    logic abys_dumper_tmp63;
    logic abys_dumper_tmp64;
    logic abys_dumper_tmp65;
    logic abys_dumper_tmp66;
    logic abys_dumper_tmp67;
    logic abys_dumper_tmp68;
    logic abys_dumper_tmp69;
    logic abys_dumper_tmp70;
    logic abys_dumper_tmp71;
    logic abys_dumper_tmp72;
    logic [12:0] abys_dumper_tmp73;
    logic [12:0] abys_dumper_tmp74;
    abys_dumper_tmp3 = amount[2'b11];
    abys_dumper_tmp4 = amount[2'b10];
    abys_dumper_tmp5 = amount[1'b1];
    abys_dumper_tmp6 = amount[1'b0];
    abys_dumper_tmp8 = data[1'b0];
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp9 = 1'b0;
    end else begin
      abys_dumper_tmp9 = abys_dumper_tmp8;
    end
    if (abys_dumper_tmp5) begin
      abys_dumper_tmp10 = 1'b0;
    end else begin
      abys_dumper_tmp10 = abys_dumper_tmp9;
    end
    abys_dumper_tmp11 = data[1'b1];
    abys_dumper_tmp12 = data[2'b10];
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp13 = abys_dumper_tmp11;
    end else begin
      abys_dumper_tmp13 = abys_dumper_tmp12;
    end
    abys_dumper_tmp14 = data[2'b11];
    abys_dumper_tmp15 = data[3'b100];
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp16 = abys_dumper_tmp14;
    end else begin
      abys_dumper_tmp16 = abys_dumper_tmp15;
    end
    if (abys_dumper_tmp5) begin
      abys_dumper_tmp17 = abys_dumper_tmp13;
    end else begin
      abys_dumper_tmp17 = abys_dumper_tmp16;
    end
    if (abys_dumper_tmp4) begin
      abys_dumper_tmp18 = abys_dumper_tmp10;
    end else begin
      abys_dumper_tmp18 = abys_dumper_tmp17;
    end
    abys_dumper_tmp19 = data[3'b101];
    abys_dumper_tmp20 = data[3'b110];
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp21 = abys_dumper_tmp19;
    end else begin
      abys_dumper_tmp21 = abys_dumper_tmp20;
    end
    abys_dumper_tmp22 = data[3'b111];
    abys_dumper_tmp23 = data[4'b1000];
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp24 = abys_dumper_tmp22;
    end else begin
      abys_dumper_tmp24 = abys_dumper_tmp23;
    end
    if (abys_dumper_tmp5) begin
      abys_dumper_tmp25 = abys_dumper_tmp21;
    end else begin
      abys_dumper_tmp25 = abys_dumper_tmp24;
    end
    abys_dumper_tmp26 = data[4'b1001];
    abys_dumper_tmp27 = data[4'b1010];
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp28 = abys_dumper_tmp26;
    end else begin
      abys_dumper_tmp28 = abys_dumper_tmp27;
    end
    abys_dumper_tmp29 = data[4'b1011];
    abys_dumper_tmp30 = data[4'b1100];
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp31 = abys_dumper_tmp29;
    end else begin
      abys_dumper_tmp31 = abys_dumper_tmp30;
    end
    if (abys_dumper_tmp5) begin
      abys_dumper_tmp32 = abys_dumper_tmp28;
    end else begin
      abys_dumper_tmp32 = abys_dumper_tmp31;
    end
    if (abys_dumper_tmp4) begin
      abys_dumper_tmp33 = abys_dumper_tmp25;
    end else begin
      abys_dumper_tmp33 = abys_dumper_tmp32;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp34 = abys_dumper_tmp18;
    end else begin
      abys_dumper_tmp34 = abys_dumper_tmp33;
    end
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp35 = abys_dumper_tmp8;
    end else begin
      abys_dumper_tmp35 = abys_dumper_tmp11;
    end
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp36 = abys_dumper_tmp12;
    end else begin
      abys_dumper_tmp36 = abys_dumper_tmp14;
    end
    if (abys_dumper_tmp5) begin
      abys_dumper_tmp37 = abys_dumper_tmp35;
    end else begin
      abys_dumper_tmp37 = abys_dumper_tmp36;
    end
    if (abys_dumper_tmp4) begin
      abys_dumper_tmp38 = 1'b0;
    end else begin
      abys_dumper_tmp38 = abys_dumper_tmp37;
    end
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp39 = abys_dumper_tmp15;
    end else begin
      abys_dumper_tmp39 = abys_dumper_tmp19;
    end
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp40 = abys_dumper_tmp20;
    end else begin
      abys_dumper_tmp40 = abys_dumper_tmp22;
    end
    if (abys_dumper_tmp5) begin
      abys_dumper_tmp41 = abys_dumper_tmp39;
    end else begin
      abys_dumper_tmp41 = abys_dumper_tmp40;
    end
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp42 = abys_dumper_tmp23;
    end else begin
      abys_dumper_tmp42 = abys_dumper_tmp26;
    end
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp43 = abys_dumper_tmp27;
    end else begin
      abys_dumper_tmp43 = abys_dumper_tmp29;
    end
    if (abys_dumper_tmp5) begin
      abys_dumper_tmp44 = abys_dumper_tmp42;
    end else begin
      abys_dumper_tmp44 = abys_dumper_tmp43;
    end
    if (abys_dumper_tmp4) begin
      abys_dumper_tmp45 = abys_dumper_tmp41;
    end else begin
      abys_dumper_tmp45 = abys_dumper_tmp44;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp46 = abys_dumper_tmp38;
    end else begin
      abys_dumper_tmp46 = abys_dumper_tmp45;
    end
    if (abys_dumper_tmp5) begin
      abys_dumper_tmp47 = abys_dumper_tmp9;
    end else begin
      abys_dumper_tmp47 = abys_dumper_tmp13;
    end
    if (abys_dumper_tmp4) begin
      abys_dumper_tmp48 = 1'b0;
    end else begin
      abys_dumper_tmp48 = abys_dumper_tmp47;
    end
    if (abys_dumper_tmp5) begin
      abys_dumper_tmp49 = abys_dumper_tmp16;
    end else begin
      abys_dumper_tmp49 = abys_dumper_tmp21;
    end
    if (abys_dumper_tmp5) begin
      abys_dumper_tmp50 = abys_dumper_tmp24;
    end else begin
      abys_dumper_tmp50 = abys_dumper_tmp28;
    end
    if (abys_dumper_tmp4) begin
      abys_dumper_tmp51 = abys_dumper_tmp49;
    end else begin
      abys_dumper_tmp51 = abys_dumper_tmp50;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp52 = abys_dumper_tmp48;
    end else begin
      abys_dumper_tmp52 = abys_dumper_tmp51;
    end
    if (abys_dumper_tmp5) begin
      abys_dumper_tmp53 = 1'b0;
    end else begin
      abys_dumper_tmp53 = abys_dumper_tmp35;
    end
    if (abys_dumper_tmp4) begin
      abys_dumper_tmp54 = 1'b0;
    end else begin
      abys_dumper_tmp54 = abys_dumper_tmp53;
    end
    if (abys_dumper_tmp5) begin
      abys_dumper_tmp55 = abys_dumper_tmp36;
    end else begin
      abys_dumper_tmp55 = abys_dumper_tmp39;
    end
    if (abys_dumper_tmp5) begin
      abys_dumper_tmp56 = abys_dumper_tmp40;
    end else begin
      abys_dumper_tmp56 = abys_dumper_tmp42;
    end
    if (abys_dumper_tmp4) begin
      abys_dumper_tmp57 = abys_dumper_tmp55;
    end else begin
      abys_dumper_tmp57 = abys_dumper_tmp56;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp58 = abys_dumper_tmp54;
    end else begin
      abys_dumper_tmp58 = abys_dumper_tmp57;
    end
    if (abys_dumper_tmp4) begin
      abys_dumper_tmp59 = 1'b0;
    end else begin
      abys_dumper_tmp59 = abys_dumper_tmp10;
    end
    if (abys_dumper_tmp4) begin
      abys_dumper_tmp60 = abys_dumper_tmp17;
    end else begin
      abys_dumper_tmp60 = abys_dumper_tmp25;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp61 = abys_dumper_tmp59;
    end else begin
      abys_dumper_tmp61 = abys_dumper_tmp60;
    end
    if (abys_dumper_tmp4) begin
      abys_dumper_tmp62 = abys_dumper_tmp37;
    end else begin
      abys_dumper_tmp62 = abys_dumper_tmp41;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp63 = 1'b0;
    end else begin
      abys_dumper_tmp63 = abys_dumper_tmp62;
    end
    if (abys_dumper_tmp4) begin
      abys_dumper_tmp64 = abys_dumper_tmp47;
    end else begin
      abys_dumper_tmp64 = abys_dumper_tmp49;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp65 = 1'b0;
    end else begin
      abys_dumper_tmp65 = abys_dumper_tmp64;
    end
    if (abys_dumper_tmp4) begin
      abys_dumper_tmp66 = abys_dumper_tmp53;
    end else begin
      abys_dumper_tmp66 = abys_dumper_tmp55;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp67 = 1'b0;
    end else begin
      abys_dumper_tmp67 = abys_dumper_tmp66;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp68 = 1'b0;
    end else begin
      abys_dumper_tmp68 = abys_dumper_tmp18;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp69 = 1'b0;
    end else begin
      abys_dumper_tmp69 = abys_dumper_tmp38;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp70 = 1'b0;
    end else begin
      abys_dumper_tmp70 = abys_dumper_tmp48;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp71 = 1'b0;
    end else begin
      abys_dumper_tmp71 = abys_dumper_tmp54;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp72 = 1'b0;
    end else begin
      abys_dumper_tmp72 = abys_dumper_tmp59;
    end
    abys_dumper_tmp73 = {abys_dumper_tmp34, abys_dumper_tmp46, abys_dumper_tmp52, abys_dumper_tmp58, abys_dumper_tmp61, abys_dumper_tmp63, abys_dumper_tmp65, abys_dumper_tmp67, abys_dumper_tmp68, abys_dumper_tmp69, abys_dumper_tmp70, abys_dumper_tmp71, abys_dumper_tmp72};
    abys_dumper_tmp74 = abys_dumper_tmp73;
    left = abys_dumper_tmp74;
  end
  always @(*)   begin
    logic abys_dumper_tmp3;
    logic abys_dumper_tmp4;
    logic abys_dumper_tmp5;
    logic abys_dumper_tmp6;
    logic abys_dumper_tmp8;
    logic abys_dumper_tmp9;
    logic abys_dumper_tmp10;
    logic abys_dumper_tmp11;
    logic abys_dumper_tmp12;
    logic abys_dumper_tmp13;
    logic abys_dumper_tmp14;
    logic abys_dumper_tmp15;
    logic abys_dumper_tmp16;
    logic abys_dumper_tmp17;
    logic abys_dumper_tmp18;
    logic abys_dumper_tmp19;
    logic abys_dumper_tmp20;
    logic abys_dumper_tmp21;
    logic abys_dumper_tmp22;
    logic abys_dumper_tmp23;
    logic abys_dumper_tmp24;
    logic abys_dumper_tmp25;
    logic abys_dumper_tmp26;
    logic abys_dumper_tmp27;
    logic abys_dumper_tmp28;
    logic abys_dumper_tmp29;
    logic abys_dumper_tmp30;
    logic abys_dumper_tmp31;
    logic abys_dumper_tmp32;
    logic abys_dumper_tmp33;
    logic abys_dumper_tmp34;
    logic abys_dumper_tmp35;
    logic abys_dumper_tmp36;
    logic abys_dumper_tmp37;
    logic abys_dumper_tmp38;
    logic abys_dumper_tmp39;
    logic abys_dumper_tmp40;
    logic abys_dumper_tmp41;
    logic abys_dumper_tmp42;
    logic abys_dumper_tmp43;
    logic abys_dumper_tmp44;
    logic abys_dumper_tmp45;
    logic abys_dumper_tmp46;
    logic abys_dumper_tmp47;
    logic abys_dumper_tmp48;
    logic abys_dumper_tmp49;
    logic abys_dumper_tmp50;
    logic abys_dumper_tmp51;
    logic abys_dumper_tmp52;
    logic abys_dumper_tmp53;
    logic abys_dumper_tmp54;
    logic abys_dumper_tmp55;
    logic abys_dumper_tmp56;
    logic abys_dumper_tmp57;
    logic abys_dumper_tmp58;
    logic abys_dumper_tmp59;
    logic abys_dumper_tmp60;
    logic abys_dumper_tmp61;
    logic abys_dumper_tmp62;
    logic abys_dumper_tmp63;
    logic abys_dumper_tmp64;
    logic abys_dumper_tmp65;
    logic abys_dumper_tmp66;
    logic abys_dumper_tmp67;
    logic abys_dumper_tmp68;
    logic abys_dumper_tmp69;
    logic abys_dumper_tmp70;
    logic abys_dumper_tmp71;
    logic abys_dumper_tmp72;
    logic [12:0] abys_dumper_tmp73;
    logic [12:0] abys_dumper_tmp74;
    abys_dumper_tmp3 = amount[2'b11];
    abys_dumper_tmp4 = amount[2'b10];
    abys_dumper_tmp5 = amount[1'b1];
    abys_dumper_tmp6 = amount[1'b0];
    abys_dumper_tmp8 = data[4'b1100];
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp9 = 1'b0;
    end else begin
      abys_dumper_tmp9 = abys_dumper_tmp8;
    end
    if (abys_dumper_tmp5) begin
      abys_dumper_tmp10 = 1'b0;
    end else begin
      abys_dumper_tmp10 = abys_dumper_tmp9;
    end
    if (abys_dumper_tmp4) begin
      abys_dumper_tmp11 = 1'b0;
    end else begin
      abys_dumper_tmp11 = abys_dumper_tmp10;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp12 = 1'b0;
    end else begin
      abys_dumper_tmp12 = abys_dumper_tmp11;
    end
    abys_dumper_tmp13 = data[4'b1011];
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp14 = abys_dumper_tmp8;
    end else begin
      abys_dumper_tmp14 = abys_dumper_tmp13;
    end
    if (abys_dumper_tmp5) begin
      abys_dumper_tmp15 = 1'b0;
    end else begin
      abys_dumper_tmp15 = abys_dumper_tmp14;
    end
    if (abys_dumper_tmp4) begin
      abys_dumper_tmp16 = 1'b0;
    end else begin
      abys_dumper_tmp16 = abys_dumper_tmp15;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp17 = 1'b0;
    end else begin
      abys_dumper_tmp17 = abys_dumper_tmp16;
    end
    abys_dumper_tmp18 = data[4'b1010];
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp19 = abys_dumper_tmp13;
    end else begin
      abys_dumper_tmp19 = abys_dumper_tmp18;
    end
    if (abys_dumper_tmp5) begin
      abys_dumper_tmp20 = abys_dumper_tmp9;
    end else begin
      abys_dumper_tmp20 = abys_dumper_tmp19;
    end
    if (abys_dumper_tmp4) begin
      abys_dumper_tmp21 = 1'b0;
    end else begin
      abys_dumper_tmp21 = abys_dumper_tmp20;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp22 = 1'b0;
    end else begin
      abys_dumper_tmp22 = abys_dumper_tmp21;
    end
    abys_dumper_tmp23 = data[4'b1001];
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp24 = abys_dumper_tmp18;
    end else begin
      abys_dumper_tmp24 = abys_dumper_tmp23;
    end
    if (abys_dumper_tmp5) begin
      abys_dumper_tmp25 = abys_dumper_tmp14;
    end else begin
      abys_dumper_tmp25 = abys_dumper_tmp24;
    end
    if (abys_dumper_tmp4) begin
      abys_dumper_tmp26 = 1'b0;
    end else begin
      abys_dumper_tmp26 = abys_dumper_tmp25;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp27 = 1'b0;
    end else begin
      abys_dumper_tmp27 = abys_dumper_tmp26;
    end
    abys_dumper_tmp28 = data[4'b1000];
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp29 = abys_dumper_tmp23;
    end else begin
      abys_dumper_tmp29 = abys_dumper_tmp28;
    end
    if (abys_dumper_tmp5) begin
      abys_dumper_tmp30 = abys_dumper_tmp19;
    end else begin
      abys_dumper_tmp30 = abys_dumper_tmp29;
    end
    if (abys_dumper_tmp4) begin
      abys_dumper_tmp31 = abys_dumper_tmp10;
    end else begin
      abys_dumper_tmp31 = abys_dumper_tmp30;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp32 = 1'b0;
    end else begin
      abys_dumper_tmp32 = abys_dumper_tmp31;
    end
    abys_dumper_tmp33 = data[3'b111];
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp34 = abys_dumper_tmp28;
    end else begin
      abys_dumper_tmp34 = abys_dumper_tmp33;
    end
    if (abys_dumper_tmp5) begin
      abys_dumper_tmp35 = abys_dumper_tmp24;
    end else begin
      abys_dumper_tmp35 = abys_dumper_tmp34;
    end
    if (abys_dumper_tmp4) begin
      abys_dumper_tmp36 = abys_dumper_tmp15;
    end else begin
      abys_dumper_tmp36 = abys_dumper_tmp35;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp37 = 1'b0;
    end else begin
      abys_dumper_tmp37 = abys_dumper_tmp36;
    end
    abys_dumper_tmp38 = data[3'b110];
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp39 = abys_dumper_tmp33;
    end else begin
      abys_dumper_tmp39 = abys_dumper_tmp38;
    end
    if (abys_dumper_tmp5) begin
      abys_dumper_tmp40 = abys_dumper_tmp29;
    end else begin
      abys_dumper_tmp40 = abys_dumper_tmp39;
    end
    if (abys_dumper_tmp4) begin
      abys_dumper_tmp41 = abys_dumper_tmp20;
    end else begin
      abys_dumper_tmp41 = abys_dumper_tmp40;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp42 = 1'b0;
    end else begin
      abys_dumper_tmp42 = abys_dumper_tmp41;
    end
    abys_dumper_tmp43 = data[3'b101];
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp44 = abys_dumper_tmp38;
    end else begin
      abys_dumper_tmp44 = abys_dumper_tmp43;
    end
    if (abys_dumper_tmp5) begin
      abys_dumper_tmp45 = abys_dumper_tmp34;
    end else begin
      abys_dumper_tmp45 = abys_dumper_tmp44;
    end
    if (abys_dumper_tmp4) begin
      abys_dumper_tmp46 = abys_dumper_tmp25;
    end else begin
      abys_dumper_tmp46 = abys_dumper_tmp45;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp47 = 1'b0;
    end else begin
      abys_dumper_tmp47 = abys_dumper_tmp46;
    end
    abys_dumper_tmp48 = data[3'b100];
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp49 = abys_dumper_tmp43;
    end else begin
      abys_dumper_tmp49 = abys_dumper_tmp48;
    end
    if (abys_dumper_tmp5) begin
      abys_dumper_tmp50 = abys_dumper_tmp39;
    end else begin
      abys_dumper_tmp50 = abys_dumper_tmp49;
    end
    if (abys_dumper_tmp4) begin
      abys_dumper_tmp51 = abys_dumper_tmp30;
    end else begin
      abys_dumper_tmp51 = abys_dumper_tmp50;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp52 = abys_dumper_tmp11;
    end else begin
      abys_dumper_tmp52 = abys_dumper_tmp51;
    end
    abys_dumper_tmp53 = data[2'b11];
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp54 = abys_dumper_tmp48;
    end else begin
      abys_dumper_tmp54 = abys_dumper_tmp53;
    end
    if (abys_dumper_tmp5) begin
      abys_dumper_tmp55 = abys_dumper_tmp44;
    end else begin
      abys_dumper_tmp55 = abys_dumper_tmp54;
    end
    if (abys_dumper_tmp4) begin
      abys_dumper_tmp56 = abys_dumper_tmp35;
    end else begin
      abys_dumper_tmp56 = abys_dumper_tmp55;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp57 = abys_dumper_tmp16;
    end else begin
      abys_dumper_tmp57 = abys_dumper_tmp56;
    end
    abys_dumper_tmp58 = data[2'b10];
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp59 = abys_dumper_tmp53;
    end else begin
      abys_dumper_tmp59 = abys_dumper_tmp58;
    end
    if (abys_dumper_tmp5) begin
      abys_dumper_tmp60 = abys_dumper_tmp49;
    end else begin
      abys_dumper_tmp60 = abys_dumper_tmp59;
    end
    if (abys_dumper_tmp4) begin
      abys_dumper_tmp61 = abys_dumper_tmp40;
    end else begin
      abys_dumper_tmp61 = abys_dumper_tmp60;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp62 = abys_dumper_tmp21;
    end else begin
      abys_dumper_tmp62 = abys_dumper_tmp61;
    end
    abys_dumper_tmp63 = data[1'b1];
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp64 = abys_dumper_tmp58;
    end else begin
      abys_dumper_tmp64 = abys_dumper_tmp63;
    end
    if (abys_dumper_tmp5) begin
      abys_dumper_tmp65 = abys_dumper_tmp54;
    end else begin
      abys_dumper_tmp65 = abys_dumper_tmp64;
    end
    if (abys_dumper_tmp4) begin
      abys_dumper_tmp66 = abys_dumper_tmp45;
    end else begin
      abys_dumper_tmp66 = abys_dumper_tmp65;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp67 = abys_dumper_tmp26;
    end else begin
      abys_dumper_tmp67 = abys_dumper_tmp66;
    end
    abys_dumper_tmp68 = data[1'b0];
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp69 = abys_dumper_tmp63;
    end else begin
      abys_dumper_tmp69 = abys_dumper_tmp68;
    end
    if (abys_dumper_tmp5) begin
      abys_dumper_tmp70 = abys_dumper_tmp59;
    end else begin
      abys_dumper_tmp70 = abys_dumper_tmp69;
    end
    if (abys_dumper_tmp4) begin
      abys_dumper_tmp71 = abys_dumper_tmp50;
    end else begin
      abys_dumper_tmp71 = abys_dumper_tmp70;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp72 = abys_dumper_tmp31;
    end else begin
      abys_dumper_tmp72 = abys_dumper_tmp71;
    end
    abys_dumper_tmp73 = {abys_dumper_tmp12, abys_dumper_tmp17, abys_dumper_tmp22, abys_dumper_tmp27, abys_dumper_tmp32, abys_dumper_tmp37, abys_dumper_tmp42, abys_dumper_tmp47, abys_dumper_tmp52, abys_dumper_tmp57, abys_dumper_tmp62, abys_dumper_tmp67, abys_dumper_tmp72};
    abys_dumper_tmp74 = abys_dumper_tmp73;
    right = abys_dumper_tmp74;
  end
  always @(*)   begin
    logic abys_dumper_tmp3;
    logic abys_dumper_tmp5;
    logic abys_dumper_tmp6;
    logic abys_dumper_tmp7;
    logic abys_dumper_tmp8;
    logic abys_dumper_tmp9;
    logic abys_dumper_tmp10;
    logic abys_dumper_tmp11;
    logic abys_dumper_tmp12;
    logic abys_dumper_tmp13;
    logic abys_dumper_tmp14;
    logic abys_dumper_tmp15;
    logic abys_dumper_tmp16;
    logic abys_dumper_tmp17;
    logic abys_dumper_tmp18;
    logic abys_dumper_tmp19;
    logic abys_dumper_tmp20;
    logic abys_dumper_tmp21;
    logic abys_dumper_tmp22;
    logic abys_dumper_tmp23;
    logic abys_dumper_tmp24;
    logic abys_dumper_tmp25;
    logic abys_dumper_tmp26;
    logic abys_dumper_tmp27;
    logic abys_dumper_tmp28;
    logic abys_dumper_tmp29;
    logic abys_dumper_tmp30;
    logic abys_dumper_tmp31;
    logic abys_dumper_tmp32;
    logic abys_dumper_tmp33;
    logic abys_dumper_tmp34;
    logic abys_dumper_tmp35;
    logic abys_dumper_tmp36;
    logic abys_dumper_tmp37;
    logic abys_dumper_tmp38;
    logic abys_dumper_tmp39;
    logic abys_dumper_tmp40;
    logic abys_dumper_tmp41;
    logic abys_dumper_tmp42;
    logic abys_dumper_tmp43;
    logic abys_dumper_tmp44;
    logic abys_dumper_tmp45;
    logic abys_dumper_tmp46;
    logic abys_dumper_tmp47;
    logic abys_dumper_tmp48;
    logic abys_dumper_tmp49;
    logic abys_dumper_tmp50;
    logic abys_dumper_tmp51;
    logic abys_dumper_tmp52;
    logic abys_dumper_tmp53;
    logic abys_dumper_tmp54;
    logic abys_dumper_tmp55;
    logic abys_dumper_tmp56;
    logic abys_dumper_tmp57;
    logic abys_dumper_tmp58;
    logic abys_dumper_tmp59;
    logic abys_dumper_tmp60;
    logic abys_dumper_tmp61;
    logic abys_dumper_tmp62;
    logic abys_dumper_tmp63;
    logic abys_dumper_tmp64;
    logic abys_dumper_tmp65;
    logic abys_dumper_tmp66;
    logic abys_dumper_tmp67;
    logic abys_dumper_tmp68;
    logic abys_dumper_tmp69;
    logic abys_dumper_tmp70;
    logic abys_dumper_tmp71;
    logic abys_dumper_tmp72;
    logic signed [12:0] abys_dumper_tmp73;
    logic signed [12:0] abys_dumper_tmp74;
    abys_dumper_tmp3 = amount[2'b11];
    abys_dumper_tmp5 = signed_data[4'b1100];
    abys_dumper_tmp6 = amount[2'b10];
    abys_dumper_tmp7 = amount[1'b1];
    abys_dumper_tmp8 = amount[1'b0];
    if (abys_dumper_tmp8) begin
      abys_dumper_tmp9 = abys_dumper_tmp5;
    end else begin
      abys_dumper_tmp9 = abys_dumper_tmp5;
    end
    if (abys_dumper_tmp7) begin
      abys_dumper_tmp10 = abys_dumper_tmp5;
    end else begin
      abys_dumper_tmp10 = abys_dumper_tmp9;
    end
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp11 = abys_dumper_tmp5;
    end else begin
      abys_dumper_tmp11 = abys_dumper_tmp10;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp12 = abys_dumper_tmp5;
    end else begin
      abys_dumper_tmp12 = abys_dumper_tmp11;
    end
    abys_dumper_tmp13 = signed_data[4'b1011];
    if (abys_dumper_tmp8) begin
      abys_dumper_tmp14 = abys_dumper_tmp5;
    end else begin
      abys_dumper_tmp14 = abys_dumper_tmp13;
    end
    if (abys_dumper_tmp7) begin
      abys_dumper_tmp15 = abys_dumper_tmp5;
    end else begin
      abys_dumper_tmp15 = abys_dumper_tmp14;
    end
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp16 = abys_dumper_tmp5;
    end else begin
      abys_dumper_tmp16 = abys_dumper_tmp15;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp17 = abys_dumper_tmp5;
    end else begin
      abys_dumper_tmp17 = abys_dumper_tmp16;
    end
    abys_dumper_tmp18 = signed_data[4'b1010];
    if (abys_dumper_tmp8) begin
      abys_dumper_tmp19 = abys_dumper_tmp13;
    end else begin
      abys_dumper_tmp19 = abys_dumper_tmp18;
    end
    if (abys_dumper_tmp7) begin
      abys_dumper_tmp20 = abys_dumper_tmp9;
    end else begin
      abys_dumper_tmp20 = abys_dumper_tmp19;
    end
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp21 = abys_dumper_tmp5;
    end else begin
      abys_dumper_tmp21 = abys_dumper_tmp20;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp22 = abys_dumper_tmp5;
    end else begin
      abys_dumper_tmp22 = abys_dumper_tmp21;
    end
    abys_dumper_tmp23 = signed_data[4'b1001];
    if (abys_dumper_tmp8) begin
      abys_dumper_tmp24 = abys_dumper_tmp18;
    end else begin
      abys_dumper_tmp24 = abys_dumper_tmp23;
    end
    if (abys_dumper_tmp7) begin
      abys_dumper_tmp25 = abys_dumper_tmp14;
    end else begin
      abys_dumper_tmp25 = abys_dumper_tmp24;
    end
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp26 = abys_dumper_tmp5;
    end else begin
      abys_dumper_tmp26 = abys_dumper_tmp25;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp27 = abys_dumper_tmp5;
    end else begin
      abys_dumper_tmp27 = abys_dumper_tmp26;
    end
    abys_dumper_tmp28 = signed_data[4'b1000];
    if (abys_dumper_tmp8) begin
      abys_dumper_tmp29 = abys_dumper_tmp23;
    end else begin
      abys_dumper_tmp29 = abys_dumper_tmp28;
    end
    if (abys_dumper_tmp7) begin
      abys_dumper_tmp30 = abys_dumper_tmp19;
    end else begin
      abys_dumper_tmp30 = abys_dumper_tmp29;
    end
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp31 = abys_dumper_tmp10;
    end else begin
      abys_dumper_tmp31 = abys_dumper_tmp30;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp32 = abys_dumper_tmp5;
    end else begin
      abys_dumper_tmp32 = abys_dumper_tmp31;
    end
    abys_dumper_tmp33 = signed_data[3'b111];
    if (abys_dumper_tmp8) begin
      abys_dumper_tmp34 = abys_dumper_tmp28;
    end else begin
      abys_dumper_tmp34 = abys_dumper_tmp33;
    end
    if (abys_dumper_tmp7) begin
      abys_dumper_tmp35 = abys_dumper_tmp24;
    end else begin
      abys_dumper_tmp35 = abys_dumper_tmp34;
    end
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp36 = abys_dumper_tmp15;
    end else begin
      abys_dumper_tmp36 = abys_dumper_tmp35;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp37 = abys_dumper_tmp5;
    end else begin
      abys_dumper_tmp37 = abys_dumper_tmp36;
    end
    abys_dumper_tmp38 = signed_data[3'b110];
    if (abys_dumper_tmp8) begin
      abys_dumper_tmp39 = abys_dumper_tmp33;
    end else begin
      abys_dumper_tmp39 = abys_dumper_tmp38;
    end
    if (abys_dumper_tmp7) begin
      abys_dumper_tmp40 = abys_dumper_tmp29;
    end else begin
      abys_dumper_tmp40 = abys_dumper_tmp39;
    end
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp41 = abys_dumper_tmp20;
    end else begin
      abys_dumper_tmp41 = abys_dumper_tmp40;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp42 = abys_dumper_tmp5;
    end else begin
      abys_dumper_tmp42 = abys_dumper_tmp41;
    end
    abys_dumper_tmp43 = signed_data[3'b101];
    if (abys_dumper_tmp8) begin
      abys_dumper_tmp44 = abys_dumper_tmp38;
    end else begin
      abys_dumper_tmp44 = abys_dumper_tmp43;
    end
    if (abys_dumper_tmp7) begin
      abys_dumper_tmp45 = abys_dumper_tmp34;
    end else begin
      abys_dumper_tmp45 = abys_dumper_tmp44;
    end
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp46 = abys_dumper_tmp25;
    end else begin
      abys_dumper_tmp46 = abys_dumper_tmp45;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp47 = abys_dumper_tmp5;
    end else begin
      abys_dumper_tmp47 = abys_dumper_tmp46;
    end
    abys_dumper_tmp48 = signed_data[3'b100];
    if (abys_dumper_tmp8) begin
      abys_dumper_tmp49 = abys_dumper_tmp43;
    end else begin
      abys_dumper_tmp49 = abys_dumper_tmp48;
    end
    if (abys_dumper_tmp7) begin
      abys_dumper_tmp50 = abys_dumper_tmp39;
    end else begin
      abys_dumper_tmp50 = abys_dumper_tmp49;
    end
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp51 = abys_dumper_tmp30;
    end else begin
      abys_dumper_tmp51 = abys_dumper_tmp50;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp52 = abys_dumper_tmp11;
    end else begin
      abys_dumper_tmp52 = abys_dumper_tmp51;
    end
    abys_dumper_tmp53 = signed_data[2'b11];
    if (abys_dumper_tmp8) begin
      abys_dumper_tmp54 = abys_dumper_tmp48;
    end else begin
      abys_dumper_tmp54 = abys_dumper_tmp53;
    end
    if (abys_dumper_tmp7) begin
      abys_dumper_tmp55 = abys_dumper_tmp44;
    end else begin
      abys_dumper_tmp55 = abys_dumper_tmp54;
    end
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp56 = abys_dumper_tmp35;
    end else begin
      abys_dumper_tmp56 = abys_dumper_tmp55;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp57 = abys_dumper_tmp16;
    end else begin
      abys_dumper_tmp57 = abys_dumper_tmp56;
    end
    abys_dumper_tmp58 = signed_data[2'b10];
    if (abys_dumper_tmp8) begin
      abys_dumper_tmp59 = abys_dumper_tmp53;
    end else begin
      abys_dumper_tmp59 = abys_dumper_tmp58;
    end
    if (abys_dumper_tmp7) begin
      abys_dumper_tmp60 = abys_dumper_tmp49;
    end else begin
      abys_dumper_tmp60 = abys_dumper_tmp59;
    end
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp61 = abys_dumper_tmp40;
    end else begin
      abys_dumper_tmp61 = abys_dumper_tmp60;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp62 = abys_dumper_tmp21;
    end else begin
      abys_dumper_tmp62 = abys_dumper_tmp61;
    end
    abys_dumper_tmp63 = signed_data[1'b1];
    if (abys_dumper_tmp8) begin
      abys_dumper_tmp64 = abys_dumper_tmp58;
    end else begin
      abys_dumper_tmp64 = abys_dumper_tmp63;
    end
    if (abys_dumper_tmp7) begin
      abys_dumper_tmp65 = abys_dumper_tmp54;
    end else begin
      abys_dumper_tmp65 = abys_dumper_tmp64;
    end
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp66 = abys_dumper_tmp45;
    end else begin
      abys_dumper_tmp66 = abys_dumper_tmp65;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp67 = abys_dumper_tmp26;
    end else begin
      abys_dumper_tmp67 = abys_dumper_tmp66;
    end
    abys_dumper_tmp68 = signed_data[1'b0];
    if (abys_dumper_tmp8) begin
      abys_dumper_tmp69 = abys_dumper_tmp63;
    end else begin
      abys_dumper_tmp69 = abys_dumper_tmp68;
    end
    if (abys_dumper_tmp7) begin
      abys_dumper_tmp70 = abys_dumper_tmp59;
    end else begin
      abys_dumper_tmp70 = abys_dumper_tmp69;
    end
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp71 = abys_dumper_tmp50;
    end else begin
      abys_dumper_tmp71 = abys_dumper_tmp70;
    end
    if (abys_dumper_tmp3) begin
      abys_dumper_tmp72 = abys_dumper_tmp31;
    end else begin
      abys_dumper_tmp72 = abys_dumper_tmp71;
    end
    abys_dumper_tmp73 = {abys_dumper_tmp12, abys_dumper_tmp17, abys_dumper_tmp22, abys_dumper_tmp27, abys_dumper_tmp32, abys_dumper_tmp37, abys_dumper_tmp42, abys_dumper_tmp47, abys_dumper_tmp52, abys_dumper_tmp57, abys_dumper_tmp62, abys_dumper_tmp67, abys_dumper_tmp72};
    abys_dumper_tmp74 = abys_dumper_tmp73;
    arithmetic_right = abys_dumper_tmp74;
  end
endmodule
