define i32 @f(i32 %secret) {
entry:
  %tobool = icmp ne i32 %secret, 0
  br i1 %tobool, label %if.then, label %if.else2

if.then:                                          ; preds = %entry
  %cmp = icmp sgt i32 3, 0
  br i1 %cmp, label %if.then1, label %if.else

if.then1:                                         ; preds = %if.then
  br label %if.end

if.else:                                          ; preds = %if.then
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then1
  %res.0 = phi i32 [ 1, %if.then1 ], [ 2, %if.else ]
  br label %if.end3

if.else2:                                         ; preds = %entry
  br label %if.end3

if.end3:                                          ; preds = %if.else2, %if.end
  %res.1 = phi i32 [ %res.0, %if.end ], [ 3, %if.else2 ]
  ret i32 %res.1
}
