; ModuleID = 'array_zero1.ca'
source_filename = "array_zero1.ca"

@0 = private unnamed_addr constant [3 x i8] c"%c\00", align 1

declare i32 @printf(i8*, ...)

define void @main() {
entry:
  %a4 = alloca [5 x [5 x [3 x [4 x i32]]]], align 4
  %a3 = alloca [5 x [3 x [4 x i32]]], align 4
  %a2 = alloca [3 x [4 x i32]], align 4
  %a1 = alloca [4 x i32], align 4
  %0 = bitcast [4 x i32]* %a1 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %0, i8 0, i64 16, i1 false)
  %1 = bitcast [3 x [4 x i32]]* %a2 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %1, i8 0, i64 48, i1 false)
  %2 = bitcast [5 x [3 x [4 x i32]]]* %a3 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %2, i8 0, i64 240, i1 false)
  %3 = bitcast [5 x [5 x [3 x [4 x i32]]]]* %a4 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %3, i8 0, i64 1200, i1 false)
  %n = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 103)
  %n1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 111)
  %n2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 111)
  %n3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 100)
  %n4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 10)
  br label %ret

ret:                                              ; preds = %entry
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #0

attributes #0 = { argmemonly nofree nosync nounwind willreturn writeonly }
