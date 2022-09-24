; ModuleID = 'box0.ca'
source_filename = "box0.ca"

declare i32 @printf(i8*, ...)

define void @main() {
entry:
  %heap = call void* @GC_malloc(i64 4)
  %ptrcast = bitcast void* %heap to i32*
  store volatile i32 0, i32* %ptrcast, align 4
  br label %ret

ret:                                              ; preds = %entry
  ret void
}

declare void* @GC_malloc(i64)
