; ModuleID = 'loop0.ca'
source_filename = "loop0.ca"

declare i32 @printf(i8*, ...)

define void @main() {
entry:
  br label %loopbb

loopbb:                                           ; preds = %loopbb, %entry
  br label %loopbb

endloopbb:                                        ; No predecessors!
  br label %ret

ret:                                              ; preds = %endloopbb
  ret void
}
