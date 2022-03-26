; ModuleID = 'while1.ca'
source_filename = "while1.ca"

declare i32 @printf(i8*, ...)

define void @main() {
entry:
  br label %condbb

condbb:                                           ; preds = %whilebb, %entry
  br i1 true, label %whilebb, label %endwhilebb

whilebb:                                          ; preds = %condbb
  br label %endwhilebb

aa:
  br label %condbb

endwhilebb:                                       ; preds = %whilebb, %condbb
  br label %ret

ret:                                              ; preds = %endwhilebb
  ret void
}
