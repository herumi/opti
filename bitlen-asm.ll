declare i32 @llvm.ctlz.i32(i32, i1)

define i32 @bitLen4(i32 %x) {
entry:
  %cmp = icmp eq i32 %x, 0
  br i1 %cmp, label %return, label %calc

calc:
  %0 = tail call i32 @llvm.ctlz.i32(i32 %x, i1 true)
  %add = sub i32 32, %0
  br label %return

return:
  %retval.0 = phi i32 [ %add, %calc ], [ 1, %entry ]
  ret i32 %retval.0
}
