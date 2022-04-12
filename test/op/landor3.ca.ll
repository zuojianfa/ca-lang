; ModuleID = 'landor3.ca'
source_filename = "landor3.ca"

@0 = private unnamed_addr constant [15 x i8] c"func1 called: \00", align 1
@1 = private unnamed_addr constant [3 x i8] c"%s\00", align 1
@2 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@3 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@4 = private unnamed_addr constant [3 x i8] c"%s\00", align 1
@5 = private unnamed_addr constant [7 x i8] c"first\0A\00", align 1
@6 = private unnamed_addr constant [3 x i8] c"%s\00", align 1
@7 = private unnamed_addr constant [8 x i8] c"second\0A\00", align 1
@8 = private unnamed_addr constant [3 x i8] c"%s\00", align 1

declare i32 @printf(i8*, ...)

define i1 @func1(i32 %seq) {
entry:
  %seq1 = alloca i32, align 4
  store volatile i32 %seq, i32* %seq1, align 4
  %retslot = alloca i1, align 1
  %n = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @1, i32 0, i32 0), i8* getelementptr inbounds ([15 x i8], [15 x i8]* @0, i32 0, i32 0))
  %load = load i32, i32* %seq1, align 4
  %n2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @2, i32 0, i32 0), i32 %load)
  %n3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i8* getelementptr inbounds ([2 x i8], [2 x i8]* @3, i32 0, i32 0))
  store i1 true, i1* %retslot, align 1
  br label %ret

afterret:                                         ; No predecessors!
  br label %ret

ret:                                              ; preds = %afterret, %entry
  %retret = load i1, i1* %retslot, align 1
  ret i1 %retret
}

define void @main() {
entry:
  br i1 true, label %thenbb, label %outbb

thenbb:                                           ; preds = %entry
  %func1 = call i1 @func1(i32 1)
  %calltmp = alloca i1, align 1
  store volatile i1 %func1, i1* %calltmp, align 1
  %v2 = load i1, i1* %calltmp, align 1
  br label %outbb

outbb:                                            ; preds = %thenbb, %entry
  %iftmp = phi i1 [ %v2, %thenbb ], [ true, %entry ]
  br i1 %iftmp, label %outbb8, label %thenbb1

thenbb1:                                          ; preds = %outbb
  br i1 false, label %thenbb2, label %outbb6

thenbb2:                                          ; preds = %thenbb1
  %func13 = call i1 @func1(i32 3)
  %calltmp4 = alloca i1, align 1
  store volatile i1 %func13, i1* %calltmp4, align 1
  %v25 = load i1, i1* %calltmp4, align 1
  br label %outbb6

outbb6:                                           ; preds = %thenbb2, %thenbb1
  %iftmp7 = phi i1 [ %v25, %thenbb2 ], [ false, %thenbb1 ]
  br label %outbb8

outbb8:                                           ; preds = %outbb6, %outbb
  %iftmp9 = phi i1 [ %iftmp7, %outbb6 ], [ %iftmp, %outbb ]
  %a = alloca i1, align 1
  store volatile i1 %iftmp9, i1* %a, align 1
  %cond = load i1, i1* %a, align 1
  br i1 %cond, label %then0, label %cond1

cond1:                                            ; preds = %outbb8
  br label %then1

then0:                                            ; preds = %outbb8
  %n = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @6, i32 0, i32 0), i8* getelementptr inbounds ([7 x i8], [7 x i8]* @5, i32 0, i32 0))
  br label %outbb11

then1:                                            ; preds = %cond1
  %n10 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @8, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @7, i32 0, i32 0))
  br label %outbb11

outbb11:                                          ; preds = %then1, %then0
  br label %ret

ret:                                              ; preds = %outbb11
  ret void
}
