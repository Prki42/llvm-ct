; guardStores case: empty then branch, stores only in else
;
; C equivalent:
;
;   void only_else_store(int secret) {
;       int x;
;       if (secret > 0) {
;           // nothing
;       } else {
;           x = 42;   // FalseReachable -> select(cond, existing_x, 42)
;       }
;   }

define void @only_else_store(i32 %secret) {
entry:
  %x = alloca i32, align 4
  %cond = icmp sgt i32 %secret, 0
  br i1 %cond, label %if.end, label %if.else

if.else:
  store i32 42, ptr %x
  br label %if.end

if.end:
  ret void
}
