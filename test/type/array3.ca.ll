; ModuleID = 'array3.ca'
source_filename = "array3.ca"

@0 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@1 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@2 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@3 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@4 = private unnamed_addr constant [3 x i8] c"%c\00", align 1

declare i32 @printf(i8*, ...)

define void @main() {
entry:
  %aa = alloca i64, align 8
  store volatile i64 0, i64* %aa, align 4
  %a1 = alloca [4 x i32], align 4
  %0 = bitcast [4 x i32]* %a1 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %0, i8 0, i64 16, i1 false)
  %a2 = alloca [3 x [4 x i32]], align 4
  %1 = bitcast [3 x [4 x i32]]* %a2 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %1, i8 0, i64 48, i1 false)
  %a3 = alloca [5 x [3 x [4 x i32]]], align 4
  %2 = bitcast [5 x [3 x [4 x i32]]]* %a3 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %2, i8 0, i64 240, i1 false)
  %a4 = alloca [5 x [5 x [3 x [4 x i32]]]], align 4
  %3 = bitcast [5 x [5 x [3 x [4 x i32]]]]* %a4 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %3, i8 0, i64 1200, i1 false)
  %a5 = alloca [3 x [5 x [5 x [3 x [4 x i32]]]]], align 4
  %4 = bitcast [3 x [5 x [5 x [3 x [4 x i32]]]]]* %a5 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %4, i8 0, i64 3600, i1 false)
  %a6 = alloca [5 x [3 x [5 x [5 x [3 x [4 x i32]]]]]], align 4
  %5 = bitcast [5 x [3 x [5 x [5 x [3 x [4 x i32]]]]]]* %a6 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %5, i8 0, i64 18000, i1 false)
  %a7 = alloca [5 x [5 x [3 x [5 x [5 x [3 x [4 x i32]]]]]]], align 4
  %6 = bitcast [5 x [5 x [3 x [5 x [5 x [3 x [4 x i32]]]]]]]* %a7 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %6, i8 0, i64 90000, i1 false)
  %a8 = alloca [3 x [4 x i32]], align 4
  %7 = bitcast [3 x [4 x i32]]* %a8 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %7, i8 0, i64 48, i1 false)
  %a9 = alloca [5 x [3 x [4 x i32]]], align 4
  %8 = bitcast [5 x [3 x [4 x i32]]]* %a9 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %8, i8 0, i64 240, i1 false)
  %a10 = alloca [5 x [5 x [3 x [4 x i32]]]], align 4
  %9 = bitcast [5 x [5 x [3 x [4 x i32]]]]* %a10 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %9, i8 0, i64 1200, i1 false)
  %a11 = alloca [3 x [5 x [5 x [3 x [4 x i32]]]]], align 4
  %10 = bitcast [3 x [5 x [5 x [3 x [4 x i32]]]]]* %a11 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %10, i8 0, i64 3600, i1 false)
  %a12 = alloca [5 x [3 x [5 x [5 x [3 x [4 x i32]]]]]], align 4
  %11 = bitcast [5 x [3 x [5 x [5 x [3 x [4 x i32]]]]]]* %a12 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %11, i8 0, i64 18000, i1 false)
  %a13 = alloca [5 x [5 x [3 x [5 x [5 x [3 x [4 x i32]]]]]]], align 4
  %12 = bitcast [5 x [5 x [3 x [5 x [5 x [3 x [4 x i32]]]]]]]* %a13 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %12, i8 0, i64 90000, i1 false)
  %a14 = alloca [4 x i32], align 4
  %13 = bitcast [4 x i32]* %a14 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %13, i8 0, i64 16, i1 false)
  %a25 = alloca [3 x [4 x i32]], align 4
  %14 = bitcast [3 x [4 x i32]]* %a25 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %14, i8 0, i64 48, i1 false)
  %a36 = alloca [5 x [3 x [4 x i32]]], align 4
  %15 = bitcast [5 x [3 x [4 x i32]]]* %a36 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %15, i8 0, i64 240, i1 false)
  %a47 = alloca [5 x [5 x [3 x [4 x i32]]]], align 4
  %16 = bitcast [5 x [5 x [3 x [4 x i32]]]]* %a47 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %16, i8 0, i64 1200, i1 false)
  %a58 = alloca [3 x [5 x [5 x [3 x [4 x i32]]]]], align 4
  %17 = bitcast [3 x [5 x [5 x [3 x [4 x i32]]]]]* %a58 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %17, i8 0, i64 3600, i1 false)
  %a69 = alloca [5 x [3 x [5 x [5 x [3 x [4 x i32]]]]]], align 4
  %18 = bitcast [5 x [3 x [5 x [5 x [3 x [4 x i32]]]]]]* %a69 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %18, i8 0, i64 18000, i1 false)
  %a710 = alloca [5 x [5 x [3 x [5 x [5 x [3 x [4 x i32]]]]]]], align 4
  %19 = bitcast [5 x [5 x [3 x [5 x [5 x [3 x [4 x i32]]]]]]]* %a710 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %19, i8 0, i64 90000, i1 false)
  %a811 = alloca [3 x [4 x i32]], align 4
  %20 = bitcast [3 x [4 x i32]]* %a811 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %20, i8 0, i64 48, i1 false)
  %a912 = alloca [5 x [3 x [4 x i32]]], align 4
  %21 = bitcast [5 x [3 x [4 x i32]]]* %a912 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %21, i8 0, i64 240, i1 false)
  %a1013 = alloca [5 x [5 x [3 x [4 x i32]]]], align 4
  %22 = bitcast [5 x [5 x [3 x [4 x i32]]]]* %a1013 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %22, i8 0, i64 1200, i1 false)
  %a1114 = alloca [3 x [5 x [5 x [3 x [4 x i32]]]]], align 4
  %23 = bitcast [3 x [5 x [5 x [3 x [4 x i32]]]]]* %a1114 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %23, i8 0, i64 3600, i1 false)
  %a1215 = alloca [5 x [3 x [5 x [5 x [3 x [4 x i32]]]]]], align 4
  %24 = bitcast [5 x [3 x [5 x [5 x [3 x [4 x i32]]]]]]* %a1215 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %24, i8 0, i64 18000, i1 false)
  %a1316 = alloca [5 x [5 x [3 x [5 x [5 x [3 x [4 x i32]]]]]]], align 4
  %25 = bitcast [5 x [5 x [3 x [5 x [5 x [3 x [4 x i32]]]]]]]* %a1316 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %25, i8 0, i64 90000, i1 false)
  %n = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 103)
  %n17 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @1, i32 0, i32 0), i8 111)
  %n18 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @2, i32 0, i32 0), i8 111)
  %n19 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @3, i32 0, i32 0), i8 100)
  %n20 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i8 10)
  br label %ret

ret:                                              ; preds = %entry
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #0

attributes #0 = { argmemonly nofree nosync nounwind willreturn writeonly }
