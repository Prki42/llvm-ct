define i32 @test_branch(i32 %secret_key, i32 %public_val) {
entry:
  %cmp1 = icmp sgt i32 %public_val, 0
  br i1 %cmp1, label %public_branch, label %end

public_branch:                                    ; preds = %entry
  br label %UnifiedReturnBlock

0:                                                ; No predecessors!
  %secret_op = xor i32 %secret_key, 123
  %cmp2 = icmp eq i32 %secret_op, 0
  br i1 %cmp2, label %secret_branch, label %end

secret_branch:                                    ; preds = %0
  br label %UnifiedReturnBlock

end:                                              ; preds = %0, %entry
  br label %UnifiedReturnBlock

UnifiedReturnBlock:                               ; preds = %end, %secret_branch, %public_branch
  %UnifiedRetVal = phi i32 [ 1, %public_branch ], [ 2, %secret_branch ], [ 0, %end ]
  ret i32 %UnifiedRetVal
}
