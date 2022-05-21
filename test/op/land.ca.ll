; ModuleID = 'land.ca'
source_filename = "land.ca"

declare i32 @printf(i8*, ...)

define void @main() {
entry:
  %e = alloca i1, align 1
  %b = alloca i32, align 4
  %a = alloca i32, align 4
  store volatile i32 1, i32* %a, align 4
  store volatile i32 2, i32* %b, align 4
  %v1 = load i32, i32* %a, align 4
  %v2 = load i32, i32* %b, align 4
  %lt = icmp slt i32 %v1, %v2
  br i1 %lt, label %thenbb, label %outbb

thenbb:                                           ; preds = %entry
  %v11 = load i32, i32* %a, align 4
  %v22 = load i32, i32* %b, align 4
  %gt = icmp sgt i32 %v11, %v22
  br label %outbb

outbb:                                            ; preds = %thenbb, %entry
  %iftmp = phi i1 [ %gt, %thenbb ], [ %lt, %entry ]
  store volatile i1 %iftmp, i1* %e, align 1
  br label %ret

ret:                                              ; preds = %outbb
  ret void
}
