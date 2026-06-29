; // pointer argument accessed directly - arr is tainted (argument)
; void store_to_arg(int *arr, int secret) {
;     arr[0] = secret;
; }
;
; // pointer argument with secret-derived index - both arr and secret tainted
; int load_from_arg(int *arr, int secret) {
;     return arr[secret & 3];
; }

; --- store_to_arg ---

define void @store_to_arg(ptr %arr, i32 %secret) {
entry:
  store i32 %secret, ptr %arr, align 4
  ret void
}

; --- load_from_arg ---

define i32 @load_from_arg(ptr %arr, i32 %secret) {
entry:
  %idx = and i32 %secret, 3
  %gep = getelementptr i32, ptr %arr, i32 %idx
  %val = load i32, ptr %gep, align 4
  ret i32 %val
}
