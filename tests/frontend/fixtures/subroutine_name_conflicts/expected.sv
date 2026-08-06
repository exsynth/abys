module top (
  input [7:0] a,
  output  logic [7:0] left_y,
  output  logic [7:0] right_y);


  left left_inst (
    .a(a),
    .y(left_y)  );
  right right_inst (
    .a(a),
    .y(right_y)  );

endmodule

module left (
  input [7:0] a,
  output  logic [7:0] y);


function automatic [7:0] transform (
  input [7:0] value
);
  begin
    logic [7:0] abys_dumper_tmp4;
    abys_dumper_tmp4 = (~value);
    transform = abys_dumper_tmp4;
  end
endfunction


  always @(*)   begin
    logic [7:0] abys_dumper_tmp3;
    abys_dumper_tmp3 = transform(a);
    y = abys_dumper_tmp3;
  end
endmodule

module right (
  input [7:0] a,
  output  logic [7:0] y);


function automatic [7:0] transform (
  input [7:0] value
);
  begin
    logic [7:0] abys_dumper_tmp5;
    abys_dumper_tmp5 = (value + 8'b1);
    transform = abys_dumper_tmp5;
  end
endfunction


  always @(*)   begin
    logic [7:0] abys_dumper_tmp3;
    abys_dumper_tmp3 = transform(a);
    y = abys_dumper_tmp3;
  end
endmodule
