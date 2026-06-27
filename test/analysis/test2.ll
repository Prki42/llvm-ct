define i32 @test_chain(i32 %key, i32 %in) {
  %a = add i32 %key, 1
  %b = mul i32 %a, 2
  %c = sub i32 %b, %in
  
  ; This branch depends on %c and %c depends on %key
  ; The analysis must pass through (key -> a -> b -> c)
  %cond = icmp eq i32 %c, 10
  br i1 %cond, label %true_label, label %false_label

true_label:
  ret i32 1
false_label:
  ret i32 0
}