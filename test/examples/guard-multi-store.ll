; guardStores case: multiple stores in the same then block (tainted + non-tainted mix)
;
; C equivalent:
;
;   void multi_store(int secret, int idx) {
;       int arr[4];
;       if (secret > 0) {
;           arr[0]   = 10;   // non-tainted ptr -> CFL semantic fix
;           arr[idx] = 20;   // tainted ptr     -> CFL semantic fix + DFL
;           arr[2]   = 30;   // non-tainted ptr -> CFL semantic fix
;       }
;   }

define void @multi_store(i32 %secret, i32 %idx) {
entry:
  %arr = alloca [4 x i32], align 4
  %cond = icmp sgt i32 %secret, 0
  br i1 %cond, label %if.then, label %if.end

if.then:
  %p0  = getelementptr i32, ptr %arr, i64 0
  store i32 10, ptr %p0                        ; non-tainted, slot 0
  %idx64       = sext i32 %idx to i64
  %tainted_ptr = getelementptr i32, ptr %arr, i64 %idx64
  store i32 20, ptr %tainted_ptr               ; tainted
  %p2  = getelementptr i32, ptr %arr, i64 2
  store i32 30, ptr %p2                        ; non-tainted, slot 2
  br label %if.end

if.end:
  ret void
}
