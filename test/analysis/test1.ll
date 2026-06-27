; input.ll
define i32 @test_branch(i32 %secret_key, i32 %public_val) {
entry:
  ; PUBLIC BRANCH
  %cmp1 = icmp sgt i32 %public_val, 0
  br i1 %cmp1, label %public_branch, label %end

public_branch:
  ret i32 1

  ; SECRET BRANCH (CFL should capture this)
  %secret_op = xor i32 %secret_key, 123
  %cmp2 = icmp eq i32 %secret_op, 0
  br i1 %cmp2, label %secret_branch, label %end

secret_branch:
  ret i32 2

end:
  ret i32 0
}