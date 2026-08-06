module top(output logic a, c);
  generate
    begin : zero_scope
      logic b;
      always_comb begin
        begin : nested_scope
          logic temporary;
          temporary = 1'b0;
          b = temporary;
        end
      end
      always_comb a = b;
    end

    begin : one_scope
      logic b;
      always_comb b = 1'b1;
      always_comb c = b;
    end
  endgenerate
endmodule
