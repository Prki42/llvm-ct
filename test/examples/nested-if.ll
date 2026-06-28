define i32 @f(i32 noundef %x, i32 noundef %y) #0 {
entry:
  %x.addr = alloca i32, align 4
  %y.addr = alloca i32, align 4
  %z = alloca i32, align 4
  store i32 %x, ptr %x.addr, align 4
  store i32 %y, ptr %y.addr, align 4
  store i32 1, ptr %z, align 4
  %0 = load i32, ptr %x.addr, align 4
  %cmp = icmp eq i32 %0, 0
  br i1 %cmp, label %if.then, label %if.else4

if.then:                                          ; preds = %entry
  %1 = load i32, ptr %y.addr, align 4
  %cmp1 = icmp eq i32 %1, 0
  br i1 %cmp1, label %if.then2, label %if.else

if.then2:                                         ; preds = %if.then
  %2 = load i32, ptr %z, align 4
  %add = add i32 %2, 2
  store i32 %add, ptr %z, align 4
  br label %if.end

if.else:                                          ; preds = %if.then
  %3 = load i32, ptr %z, align 4
  %add3 = add i32 %3, 1
  store i32 %add3, ptr %z, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then2
  br label %if.end10

if.else4:                                         ; preds = %entry
  %4 = load i32, ptr %y.addr, align 4
  %cmp5 = icmp eq i32 %4, 0
  br i1 %cmp5, label %if.then6, label %if.else7

if.then6:                                         ; preds = %if.else4
  %5 = load i32, ptr %z, align 4
  %sub = sub i32 %5, 2
  store i32 %sub, ptr %z, align 4
  br label %if.end9

if.else7:                                         ; preds = %if.else4
  %6 = load i32, ptr %z, align 4
  %sub8 = sub i32 %6, 1
  store i32 %sub8, ptr %z, align 4
  br label %if.end9

if.end9:                                          ; preds = %if.else7, %if.then6
  br label %if.end10

if.end10:                                         ; preds = %if.end9, %if.end
  %7 = load i32, ptr %z, align 4
  ret i32 %7
}

; int f(int x, int y) {
;   int z = 1;
;   if (x == 0) {
;     if (y == 0)
;       z += 2;
;     else
;       z += 1;
;   } else {
;     if (y == 0)
;       z -= 2;
;     else
;       z -= 1;
;   }
; 
;   return z;
; }
