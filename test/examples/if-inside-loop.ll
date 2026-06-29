; ModuleID = 'test/examples/if-inside-loop.ll'
source_filename = "test/examples/if-inside-loop.ll"

define i32 @f(i32 noundef %x) {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %y.0 = phi i32 [ 0, %entry ], [ %y.1, %for.inc ]
  %i.0 = phi i32 [ 0, %entry ], [ %inc2, %for.inc ]
  %cmp = icmp slt i32 %i.0, 10
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %add = add i32 %x, %i.0
  %cmp1 = icmp eq i32 %add, 2
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %inc = add i32 %y.0, 1
  br label %if.end

if.else:                                          ; preds = %for.body
  %dec = add i32 %y.0, -1
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %y.1 = phi i32 [ %inc, %if.then ], [ %dec, %if.else ]
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %inc2 = add i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret i32 %y.0
}
