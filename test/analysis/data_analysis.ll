; RUN: opt --load-pass-plugin=%plugin --passes="print<data>" -disable-output %s 2>&1 | FileCheck %s

; // secret stored into a local, then loaded back - both ops are sensitive
; int simple(int secret) {
;     int x;
;     x = secret;
;     return x;
; }
;
; // array accessed via a secret-derived index - only the load is sensitive,
; // the constant-index init stores are not
; int array_access(int secret) {
;     int arr[4] = {10, 20, 30, 40};
;     return arr[secret & 3];
; }

; --- simple ---

; CHECK-LABEL: Data objects for simple:
; CHECK:       store i32 %secret, ptr %x
; CHECK-NEXT:    base:     %x = alloca i32
; CHECK-NEXT:    form:     i32
; CHECK-NEXT:    size:     4 bytes
; CHECK-NEXT:    interval: [0, 4)
; CHECK:       %loaded = load i32, ptr %x
; CHECK-NEXT:    base:     %x = alloca i32
; CHECK-NEXT:    form:     i32
; CHECK-NEXT:    size:     4 bytes
; CHECK-NEXT:    interval: [0, 4)

define i32 @simple(i32 %secret) {
entry:
  %x = alloca i32, align 4
  store i32 %secret, ptr %x, align 4
  %loaded = load i32, ptr %x, align 4
  ret i32 %loaded
}

; --- array_access ---

; CHECK-LABEL: Data objects for array_access:
; CHECK:       %val = load i32, ptr %gep
; CHECK-NEXT:    base:     %arr = alloca [4 x i32]
; CHECK-NEXT:    form:     [4 x i32]
; CHECK-NEXT:    size:     16 bytes
; CHECK-NEXT:    interval: [0, 16)

define i32 @array_access(i32 %secret) {
entry:
  %arr = alloca [4 x i32], align 16
  ; constant-index stores - not tainted, should not appear in output
  %p0 = getelementptr [4 x i32], ptr %arr, i32 0, i32 0
  store i32 10, ptr %p0, align 4
  %p1 = getelementptr [4 x i32], ptr %arr, i32 0, i32 1
  store i32 20, ptr %p1, align 4
  %p2 = getelementptr [4 x i32], ptr %arr, i32 0, i32 2
  store i32 30, ptr %p2, align 4
  %p3 = getelementptr [4 x i32], ptr %arr, i32 0, i32 3
  store i32 40, ptr %p3, align 4
  ; secret-derived index - the resulting pointer is tainted
  %idx = and i32 %secret, 3
  %gep = getelementptr [4 x i32], ptr %arr, i32 0, i32 %idx
  %val = load i32, ptr %gep, align 4
  ret i32 %val
}
