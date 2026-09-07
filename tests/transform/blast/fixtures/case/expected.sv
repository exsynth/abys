module top (
  input [1:0] select,
  input [3:0] a,
  input [3:0] b,
  input [3:0] c,
  output  logic [3:0] y);

  logic [3:0] abys_transformer_tmp0;
  logic \abys_transformer_tmp1[0] ;
  logic \abys_transformer_tmp1[1] ;
  logic \abys_transformer_tmp1[2] ;
  logic \abys_transformer_tmp1[3] ;
  logic \abys_transformer_tmp2[0] ;
  logic \abys_transformer_tmp2[1] ;
  logic \abys_transformer_tmp2[2] ;
  logic \abys_transformer_tmp2[3] ;
  logic \abys_transformer_tmp3[0] ;
  logic \abys_transformer_tmp3[1] ;
  logic \abys_transformer_tmp3[2] ;
  logic \abys_transformer_tmp3[3] ;
  logic \abys_transformer_tmp4[0] ;
  logic \abys_transformer_tmp4[1] ;


  always @(*)   begin
    logic abys_dumper_tmp12;
    logic [1:0] abys_dumper_tmp4;
    logic abys_dumper_tmp6;
    logic abys_dumper_tmp9;
    logic abys_dumper_tmp16;
    logic abys_dumper_tmp20;
    logic abys_dumper_tmp24;
    abys_dumper_tmp4 = {\abys_transformer_tmp4[1] , \abys_transformer_tmp4[0] };
    abys_dumper_tmp6 = (abys_dumper_tmp4 == 2'b0);
    abys_dumper_tmp9 = (abys_dumper_tmp4 == 2'b1);
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp12 = \abys_transformer_tmp2[0] ;
    end
    else if (abys_dumper_tmp9) begin
      abys_dumper_tmp12 = \abys_transformer_tmp1[0] ;
    end
    else begin
      abys_dumper_tmp12 = \abys_transformer_tmp3[0] ;
    end
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp16 = \abys_transformer_tmp2[1] ;
    end
    else if (abys_dumper_tmp9) begin
      abys_dumper_tmp16 = \abys_transformer_tmp1[1] ;
    end
    else begin
      abys_dumper_tmp16 = \abys_transformer_tmp3[1] ;
    end
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp20 = \abys_transformer_tmp2[2] ;
    end
    else if (abys_dumper_tmp9) begin
      abys_dumper_tmp20 = \abys_transformer_tmp1[2] ;
    end
    else begin
      abys_dumper_tmp20 = \abys_transformer_tmp3[2] ;
    end
    if (abys_dumper_tmp6) begin
      abys_dumper_tmp24 = \abys_transformer_tmp2[3] ;
    end
    else if (abys_dumper_tmp9) begin
      abys_dumper_tmp24 = \abys_transformer_tmp1[3] ;
    end
    else begin
      abys_dumper_tmp24 = \abys_transformer_tmp3[3] ;
    end
    abys_transformer_tmp0[0] = abys_dumper_tmp12;
    abys_transformer_tmp0[1] = abys_dumper_tmp16;
    abys_transformer_tmp0[2] = abys_dumper_tmp20;
    abys_transformer_tmp0[3] = abys_dumper_tmp24;
  end
  always @(*) begin
    y = {abys_transformer_tmp0[3], abys_transformer_tmp0[2], abys_transformer_tmp0[1], abys_transformer_tmp0[0]};
  end
  always @(*) begin
    \abys_transformer_tmp1[0]  = b[0];
    \abys_transformer_tmp1[1]  = b[1];
    \abys_transformer_tmp1[2]  = b[2];
    \abys_transformer_tmp1[3]  = b[3];
  end
  always @(*) begin
    \abys_transformer_tmp2[0]  = a[0];
    \abys_transformer_tmp2[1]  = a[1];
    \abys_transformer_tmp2[2]  = a[2];
    \abys_transformer_tmp2[3]  = a[3];
  end
  always @(*) begin
    \abys_transformer_tmp3[0]  = c[0];
    \abys_transformer_tmp3[1]  = c[1];
    \abys_transformer_tmp3[2]  = c[2];
    \abys_transformer_tmp3[3]  = c[3];
  end
  always @(*) begin
    \abys_transformer_tmp4[0]  = select[0];
    \abys_transformer_tmp4[1]  = select[1];
  end
endmodule
