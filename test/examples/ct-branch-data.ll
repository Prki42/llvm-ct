; Combined CTBranchPass + CTDataPass test
;
;   void store_in_branch(int secret, int idx) {
;       int arr[4];
;       if (secret > 0) {
;           arr[idx] = 99;   // tainted pointer  -> DFL (CTDataPass)
;           arr[0]   = 42;   // constant pointer -> CFL semantic fix (CTBranchPass guardStores)
;       }
;   }

define void @store_in_branch(i32 %secret, i32 %idx) {
entry:
  %arr = alloca [4 x i32], align 4
  %cond = icmp sgt i32 %secret, 0
  br i1 %cond, label %if.then, label %if.end

if.then:
  %idx64 = sext i32 %idx to i64
  %tainted_ptr = getelementptr i32, ptr %arr, i64 %idx64
  store i32 99, ptr %tainted_ptr           ; tainted ptr, DFL applies
  store i32 42, ptr %arr                   ; non-tainted ptr, CFL semantic fix applies
  br label %if.end

if.end:
  ret void
}
