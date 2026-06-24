; RUN: %opt --load-pass-plugin=%plugin --passes="print<arg-dep>" -disable-output %s 2>&1 | %FileCheck %s

; CHECK: ArgDep for f
; 
; CHECK: Branches:
; CHECK-NEXT: br i1 %cmp4
; CHECK: br i1 %exitcond.not
; CHECK-NOT: br i1
; 
; CHECK: Values:
; CHECK-NEXT: i32 %x
; CHECK-NEXT: i32 %y
; CHECK-NEXT: %cmp4 =
; CHECK-NEXT: %y.addr.0.lcssa =
; CHECK-NEXT: %y.addr.05 =
; CHECK-NEXT: %mul =
; CHECK-NEXT: %exitcond.not =
; CHECK-NOT{LITERAL}: =

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
