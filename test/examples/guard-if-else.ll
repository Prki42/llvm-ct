; guardStores case: both then AND else branches have stores
;
; C equivalent:
;
;   void if_else_stores(int secret) {
;       int x, y;
;       if (secret > 0) {
;           x = 1;   // TrueReachable  -> select(cond, 1,        existing_x)
;       } else {
;           y = 2;   // FalseReachable -> select(cond, existing_y, 2)
;       }
;   }

define void @if_else_stores(i32 %secret) {
entry:
  %x = alloca i32, align 4
  %y = alloca i32, align 4
  %cond = icmp sgt i32 %secret, 0
  br i1 %cond, label %if.then, label %if.else

if.then:
  store i32 1, ptr %x
  br label %if.end

if.else:
  store i32 2, ptr %y
  br label %if.end

if.end:
  ret void
}
