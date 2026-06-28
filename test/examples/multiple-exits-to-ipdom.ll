define i32 @f(i32 %x, i32 %y) {
entry:
  %tobool = icmp ne i32 %x, 0
  br i1 %tobool, label %if.then, label %if.end2

if.then:                                          ; preds = %entry
  %cmp = icmp sgt i32 %y, 0
  br i1 %cmp, label %if.then1, label %if.end

if.then1:                                         ; preds = %if.then
  br label %return

if.end:                                           ; preds = %if.then
  br label %return

if.end2:                                          ; preds = %entry
  br label %return

return:                                           ; preds = %if.end2, %if.end, %if.then1
  %retval.0 = phi i32 [ 1, %if.then1 ], [ 2, %if.end ], [ 0, %if.end2 ]
  ret i32 %retval.0
}

; int f(int x, int y) {
;   if (x) {
;     if (y > 0)
;       return 1;
;     return 2;
;   }
;   return 0;
; }

