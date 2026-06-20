define i32 @f(i32 %x, i32 %y) #0 {
entry:
  %cmp4 = icmp sgt i32 %x, 1
  br i1 %cmp4, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.body, %entry
  %y.addr.0.lcssa = phi i32 [ %y, %entry ], [ %mul, %for.body ]
  ret i32 %y.addr.0.lcssa

for.body:                                         ; preds = %entry, %for.body
  %i.06 = phi i32 [ %inc, %for.body ], [ 1, %entry ]
  %y.addr.05 = phi i32 [ %mul, %for.body ], [ %y, %entry ]
  %mul = mul nsw i32 %i.06, %y.addr.05
  %inc = add nuw nsw i32 %i.06, 1
  %exitcond.not = icmp eq i32 %inc, %x
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}
