define i32 @f(i32 %x, i32 %y) {
entry:
  %rem = srem i32 %y, 2
  %cmp = icmp eq i32 %rem, 0
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %if.then
  %res.0 = phi i32 [ 0, %if.then ], [ %add, %for.inc ]
  %i.0 = phi i32 [ 0, %if.then ], [ %inc, %for.inc ]
  %cmp1 = icmp slt i32 %i.0, %x
  br i1 %cmp1, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %div = sdiv i32 %y, 2
  %add = add i32 %res.0, %div
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  br label %if.end

if.else:                                          ; preds = %entry
  br label %for.cond3

for.cond3:                                        ; preds = %for.inc7, %if.else
  %res.2 = phi i32 [ 0, %if.else ], [ %add6, %for.inc7 ]
  %i2.0 = phi i32 [ 0, %if.else ], [ %inc8, %for.inc7 ]
  %cmp4 = icmp slt i32 %i2.0, %x
  br i1 %cmp4, label %for.body5, label %for.end9

for.body5:                                        ; preds = %for.cond3
  %add6 = add i32 %res.2, %y
  br label %for.inc7

for.inc7:                                         ; preds = %for.body5
  %inc8 = add i32 %i2.0, 1
  br label %for.cond3

for.end9:                                         ; preds = %for.cond3
  br label %if.end

if.end:                                           ; preds = %for.end9, %for.end
  %res.1 = phi i32 [ %res.0, %for.end ], [ %res.2, %for.end9 ]
  ret i32 %res.1
}

; int f(int x, int y) {
;   int res = 0;
;   if (y % 2 == 0) {
;     for (int i = 0; i < x; i++) {
;       res += y / 2;
;     }
;   } else {
;     for (int i = 0; i < x; i++) {
;       res += y;
;     }
;   }
;   return res;
; }
