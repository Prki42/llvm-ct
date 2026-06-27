; RUN: %opt --load-pass-plugin=%plugin --passes="print<arg-dep>" -disable-output %s 2>&1 | %FileCheck %s

; CHECK: ArgDep for f
; 
; CHECK: Branches:
; CHECK-NOT: br i1
; 
; CHECK: Values:
; CHECK-NEXT: i32 %x
; CHECK-NEXT: i32 %y
; CHECK-NEXT: %x.addr =
; CHECK-NEXT: %y.addr =
; CHECK-NEXT: %z =
; CHECK-NEXT: %0 =
; CHECK-NEXT: %1 =
; CHECK-NEXT: %mul =
; CHECK-NEXT: %2 =
; CHECK-NOT{LITERAL}: =

define i32 @f(i32 %x, i32 %y) {
entry:
  %x.addr = alloca i32, align 4
  %y.addr = alloca i32, align 4
  %z = alloca i32, align 4

  store i32 %x, ptr %x.addr, align 4
  store i32 %y, ptr %y.addr, align 4

  %0 = load i32, ptr %x.addr, align 4
  %1 = load i32, ptr %y.addr, align 4

  %mul = mul nsw i32 %0, %1
  store i32 %mul, ptr %z, align 4

  %2 = load i32, ptr %z, align 4
  ret i32 %2
}
