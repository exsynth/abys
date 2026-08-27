module top (
  input [3:0] a,
  input select,
  output  logic [3:0] y);

  logic [3:0] child_y;
  logic [3:0] abys_transformer_tmp0;
  logic \abys_transformer_tmp1[0] ;
  logic \abys_transformer_tmp1[1] ;
  logic \abys_transformer_tmp1[2] ;
  logic \abys_transformer_tmp1[3] ;
  logic \abys_transformer_tmp2[0] ;
  logic \abys_transformer_tmp2[1] ;
  logic \abys_transformer_tmp2[2] ;
  logic \abys_transformer_tmp2[3] ;

  child u_child (
    .a(a),
    .y(child_y)  );

  always @(*)   begin
    if (select) begin
      abys_transformer_tmp0[0] = \abys_transformer_tmp2[0] ;
    end else begin
      abys_transformer_tmp0[0] = \abys_transformer_tmp1[0] ;
    end
    if (select) begin
      abys_transformer_tmp0[1] = \abys_transformer_tmp2[1] ;
    end else begin
      abys_transformer_tmp0[1] = \abys_transformer_tmp1[1] ;
    end
    if (select) begin
      abys_transformer_tmp0[2] = \abys_transformer_tmp2[2] ;
    end else begin
      abys_transformer_tmp0[2] = \abys_transformer_tmp1[2] ;
    end
    if (select) begin
      abys_transformer_tmp0[3] = \abys_transformer_tmp2[3] ;
    end else begin
      abys_transformer_tmp0[3] = \abys_transformer_tmp1[3] ;
    end
  end
  always @(*) begin
    y = {abys_transformer_tmp0[3], abys_transformer_tmp0[2], abys_transformer_tmp0[1], abys_transformer_tmp0[0]};
  end
  always @(*) begin
    \abys_transformer_tmp1[0]  = a[0];
    \abys_transformer_tmp1[1]  = a[1];
    \abys_transformer_tmp1[2]  = a[2];
    \abys_transformer_tmp1[3]  = a[3];
  end
  always @(*) begin
    \abys_transformer_tmp2[0]  = child_y[0];
    \abys_transformer_tmp2[1]  = child_y[1];
    \abys_transformer_tmp2[2]  = child_y[2];
    \abys_transformer_tmp2[3]  = child_y[3];
  end
endmodule

module child (
  input [3:0] a,
  output  logic [3:0] y);

  logic [3:0] abys_transformer_tmp0;
  logic \abys_transformer_tmp1[0] ;
  logic \abys_transformer_tmp1[1] ;
  logic \abys_transformer_tmp1[2] ;
  logic \abys_transformer_tmp1[3] ;


  always @(*)   begin
    logic abys_dumper_tmp9;
    logic [3:0] abys_dumper_tmp6;
    logic [3:0] abys_dumper_tmp8;
    logic abys_dumper_tmp10;
    logic abys_dumper_tmp12;
    logic abys_dumper_tmp14;
    abys_dumper_tmp6 = {\abys_transformer_tmp1[3] , \abys_transformer_tmp1[2] , \abys_transformer_tmp1[1] , \abys_transformer_tmp1[0] };
    abys_dumper_tmp8 = (abys_dumper_tmp6 + 4'b1);
    abys_dumper_tmp9 = ((abys_dumper_tmp8 >> (1'b0)) & {1{1'b1}});
    abys_dumper_tmp10 = ((abys_dumper_tmp8 >> (1'b1)) & {1{1'b1}});
    abys_dumper_tmp12 = ((abys_dumper_tmp8 >> (2'b10)) & {1{1'b1}});
    abys_dumper_tmp14 = ((abys_dumper_tmp8 >> (2'b11)) & {1{1'b1}});
    abys_transformer_tmp0[0] = abys_dumper_tmp9;
    abys_transformer_tmp0[1] = abys_dumper_tmp10;
    abys_transformer_tmp0[2] = abys_dumper_tmp12;
    abys_transformer_tmp0[3] = abys_dumper_tmp14;
  end
  always @(*) begin
    y = {abys_transformer_tmp0[3], abys_transformer_tmp0[2], abys_transformer_tmp0[1], abys_transformer_tmp0[0]};
  end
  always @(*) begin
    \abys_transformer_tmp1[0]  = a[0];
    \abys_transformer_tmp1[1]  = a[1];
    \abys_transformer_tmp1[2]  = a[2];
    \abys_transformer_tmp1[3]  = a[3];
  end
endmodule
