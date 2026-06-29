; RUN: %opt --load-pass-plugin=%plugin --passes="print<data>" -disable-output %s 2>&1 | %FileCheck %s

; typedef struct { int x; int y; } Point;
;
; int f(int secret) {
;     Point p;
;     p.x = secret;   // store through tainted GEP into struct - sensitive
;     return p.x;     // load through same GEP - sensitive
;                     // note: interval covers full struct [0, 8) even though
;                     //       only field x (bytes [0, 4)) is actually accessed
; }

%Point = type { i32, i32 }

; CHECK-LABEL: Data objects for f:
; CHECK:       store i32 %secret, ptr %x_ptr
; CHECK-NEXT:    base:     %p = alloca %Point
; CHECK-NEXT:    form:     %Point
; CHECK-NEXT:    size:     8 bytes
; CHECK-NEXT:    interval: [0, 8
; CHECK:       %loaded = load i32, ptr %x_ptr
; CHECK-NEXT:    base:     %p = alloca %Point
; CHECK-NEXT:    form:     %Point
; CHECK-NEXT:    size:     8 bytes
; CHECK-NEXT:    interval: [0, 8

define i32 @f(i32 %secret) {
entry:
  %p     = alloca %Point, align 4
  %x_ptr = getelementptr %Point, ptr %p, i32 0, i32 0
  store i32 %secret, ptr %x_ptr, align 4
  %loaded = load i32, ptr %x_ptr, align 4
  ret i32 %loaded
}
