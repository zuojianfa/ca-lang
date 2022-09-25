; ModuleID = 'type_zero1.ca'
source_filename = "type_zero1.ca"

@0 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@1 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@2 = private unnamed_addr constant [4 x i8] c"%ld\00", align 1
@3 = private unnamed_addr constant [3 x i8] c"%u\00", align 1
@4 = private unnamed_addr constant [4 x i8] c"%lu\00", align 1
@5 = private unnamed_addr constant [3 x i8] c"%f\00", align 1
@6 = private unnamed_addr constant [4 x i8] c"%lf\00", align 1
@7 = private unnamed_addr constant [4 x i8] c"%1d\00", align 1

declare i32 @printf(i8*, ...)

define void @main() {
entry:
  %a11 = alloca i8, align 1
  %a10 = alloca i8, align 1
  %a9 = alloca i1, align 1
  %a8 = alloca double, align 8
  %a7 = alloca float, align 4
  %a6 = alloca i64, align 8
  %a5 = alloca i32, align 4
  %a4 = alloca i8, align 1
  %a3 = alloca i64, align 8
  %a2 = alloca i32, align 4
  %a1 = alloca i8, align 1
  call void @llvm.memset.p0i8.i64(i8* align 1 %a1, i8 0, i64 1, i1 false)
  %0 = bitcast i32* %a2 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %0, i8 0, i64 4, i1 false)
  %1 = bitcast i64* %a3 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %1, i8 0, i64 8, i1 false)
  call void @llvm.memset.p0i8.i64(i8* align 1 %a4, i8 0, i64 1, i1 false)
  %2 = bitcast i32* %a5 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %2, i8 0, i64 4, i1 false)
  %3 = bitcast i64* %a6 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %3, i8 0, i64 8, i1 false)
  %4 = bitcast float* %a7 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %4, i8 0, i64 4, i1 false)
  %5 = bitcast double* %a8 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %5, i8 0, i64 8, i1 false)
  %6 = bitcast i1* %a9 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 1 %6, i8 0, i64 1, i1 false)
  call void @llvm.memset.p0i8.i64(i8* align 1 %a10, i8 0, i64 1, i1 false)
  call void @llvm.memset.p0i8.i64(i8* align 1 %a11, i8 0, i64 1, i1 false)
  %load = load i8, i8* %a1, align 1
  %n = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 %load)
  %n1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 32)
  %load2 = load i32, i32* %a2, align 4
  %n3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @1, i32 0, i32 0), i32 %load2)
  %n4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 32)
  %load5 = load i64, i64* %a3, align 4
  %n6 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @2, i32 0, i32 0), i64 %load5)
  %n7 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 32)
  %load8 = load i8, i8* %a4, align 1
  %n9 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 %load8)
  %n10 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 32)
  %load11 = load i32, i32* %a5, align 4
  %n12 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @3, i32 0, i32 0), i32 %load11)
  %n13 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 32)
  %load14 = load i64, i64* %a6, align 4
  %n15 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @4, i32 0, i32 0), i64 %load14)
  %n16 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 32)
  %load17 = load float, float* %a7, align 4
  %7 = fpext float %load17 to double
  %n18 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0), double %7)
  %n19 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 32)
  %load20 = load double, double* %a8, align 8
  %n21 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @6, i32 0, i32 0), double %load20)
  %n22 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 32)
  %load23 = load i1, i1* %a9, align 1
  %8 = zext i1 %load23 to i32
  %n24 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @7, i32 0, i32 0), i32 %8)
  %n25 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 32)
  %load26 = load i8, i8* %a10, align 1
  %n27 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 %load26)
  %n28 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 32)
  %load29 = load i8, i8* %a11, align 1
  %n30 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 %load29)
  %n31 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 32)
  %n32 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 103)
  %n33 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 111)
  %n34 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 111)
  %n35 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 100)
  %n36 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 10)
  br label %ret

ret:                                              ; preds = %entry
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #0

attributes #0 = { argmemonly nofree nosync nounwind willreturn writeonly }
