define i32 @f(i32 %x, i32 %y) {
entry:
  %tobool = icmp ne i32 %y, 0
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %fallback

if.end:                                           ; preds = %entry
  %cmp = icmp eq i32 %x, 0
  br i1 %cmp, label %if.then1, label %if.end2

if.then1:                                         ; preds = %if.end
  br label %return

if.end2:                                          ; preds = %if.end
  br label %fallback

fallback:                                         ; preds = %if.end2, %if.then
  br label %return

return:                                           ; preds = %fallback, %if.then1
  %retval.0 = phi i32 [ 0, %fallback ], [ 1, %if.then1 ]
  ret i32 %retval.0
}

; int f(int x, int y) {
;   if (y)
;     goto fallback;
; 
;   if (x == 0)
;     return 1;
; 
; fallback:
;   return 0;
; }
