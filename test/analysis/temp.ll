define i32 @temp(i32 %x) {
  %cmp = icmp sgt i32 %x, 0
  br i1 %cmp, label %then, label %else
then:
  ret i32 1
else:
  ret i32 0
}
