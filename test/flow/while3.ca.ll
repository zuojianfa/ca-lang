; ModuleID = 'while3.ca'
source_filename = "while3.ca"

@0 = private unnamed_addr constant [16 x i8] c"while finished\0A\00", align 1
@1 = private unnamed_addr constant [3 x i8] c"%s\00", align 1
@2 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@3 = private unnamed_addr constant [3 x i8] c"%c\00", align 1

declare i32 @printf(i8*, ...)

define void @main() {
entry:
  %i = alloca i32, align 4
  store volatile i32 0, i32* %i, align 4
  br label %condbb

condbb:                                           ; preds = %outbb, %entry
  br i1 true, label %whilebb, label %endwhilebb

whilebb:                                          ; preds = %condbb
  %0 = load i32, i32* %i, align 4
  %add = add i32 %0, 1
  store volatile i32 %add, i32* %i, align 4
  %v1 = load i32, i32* %i, align 4
  %gt = icmp sgt i32 %v1, 10
  br i1 %gt, label %then0, label %cond1

cond1:                                            ; preds = %whilebb
  br label %outbb

then0:                                            ; preds = %whilebb
  %n = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @1, i32 0, i32 0), i8* getelementptr inbounds ([16 x i8], [16 x i8]* @0, i32 0, i32 0))
  br label %endwhilebb

extra:                                            ; No predecessors!
  %a = alloca i32, align 4
  store volatile i32 2, i32* %a, align 4
  %1 = load i32, i32* %a, align 4
  %add1 = add i32 %1, 2
  store volatile i32 %add1, i32* %a, align 4
  br label %outbb

outbb:                                            ; preds = %extra, %cond1
  %load = load i32, i32* %i, align 4
  %n2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @2, i32 0, i32 0), i32 %load)
  %n3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @3, i32 0, i32 0), i8 32)
  br label %condbb

endwhilebb:                                       ; preds = %then0, %condbb
  br label %ret

ret:                                              ; preds = %endwhilebb
  ret void
}
