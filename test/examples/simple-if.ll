define i32 @f(i32 noundef %x) #0 {
entry:
  %x.addr = alloca i32, align 4
  %y = alloca i32, align 4
  store i32 %x, ptr %x.addr, align 4
  store i32 1, ptr %y, align 4
  %0 = load i32, ptr %x.addr, align 4
  %cmp = icmp eq i32 %0, 0
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %1 = load i32, ptr %y, align 4
  %inc = add i32 %1, 1
  store i32 %inc, ptr %y, align 4
  br label %if.end

if.else:                                          ; preds = %entry
  %2 = load i32, ptr %y, align 4
  %dec = add i32 %2, -1
  store i32 %dec, ptr %y, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %3 = load i32, ptr %y, align 4
  ret i32 %3
}

; int f(int x) {
;   int y = 1;
;   if (x == 0) {
;     y++;
;   } else {
;     y--;
;   }
; 
;   return y;
; }
