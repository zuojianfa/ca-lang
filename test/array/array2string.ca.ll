; ModuleID = 'array2string.ca'
source_filename = "array2string.ca"

@constarray = internal constant [11 x i8] c"Hello CA\0AC\00", align 4
@0 = private unnamed_addr constant [2 x i8] c"[\00", align 1
@1 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@2 = private unnamed_addr constant [3 x i8] c", \00", align 1
@3 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@4 = private unnamed_addr constant [3 x i8] c", \00", align 1
@5 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@6 = private unnamed_addr constant [3 x i8] c", \00", align 1
@7 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@8 = private unnamed_addr constant [3 x i8] c", \00", align 1
@9 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@10 = private unnamed_addr constant [3 x i8] c", \00", align 1
@11 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@12 = private unnamed_addr constant [3 x i8] c", \00", align 1
@13 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@14 = private unnamed_addr constant [3 x i8] c", \00", align 1
@15 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@16 = private unnamed_addr constant [3 x i8] c", \00", align 1
@17 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@18 = private unnamed_addr constant [3 x i8] c", \00", align 1
@19 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@20 = private unnamed_addr constant [3 x i8] c", \00", align 1
@21 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@22 = private unnamed_addr constant [2 x i8] c"]\00", align 1
@23 = private unnamed_addr constant [3 x i8] c"%c\00", align 1

declare i32 @printf(i8*, ...)

define void @main(...) {
entry:
  %a = alloca [11 x i8], align 1
  %0 = bitcast [11 x i8]* %a to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %0, i8* align 1 getelementptr inbounds ([11 x i8], [11 x i8]* @constarray, i32 0, i32 0), i64 11, i1 false)
  %load = load [11 x i8], [11 x i8]* %a, align 1
  %n = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @0, i32 0, i32 0))
  %1 = extractvalue [11 x i8] %load, 0
  %n1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @1, i32 0, i32 0), i8 %1)
  %n2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @2, i32 0, i32 0))
  %2 = extractvalue [11 x i8] %load, 1
  %n3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @3, i32 0, i32 0), i8 %2)
  %n4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0))
  %3 = extractvalue [11 x i8] %load, 2
  %n5 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0), i8 %3)
  %n6 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @6, i32 0, i32 0))
  %4 = extractvalue [11 x i8] %load, 3
  %n7 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @7, i32 0, i32 0), i8 %4)
  %n8 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @8, i32 0, i32 0))
  %5 = extractvalue [11 x i8] %load, 4
  %n9 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @9, i32 0, i32 0), i8 %5)
  %n10 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @10, i32 0, i32 0))
  %6 = extractvalue [11 x i8] %load, 5
  %n11 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @11, i32 0, i32 0), i8 %6)
  %n12 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @12, i32 0, i32 0))
  %7 = extractvalue [11 x i8] %load, 6
  %n13 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @13, i32 0, i32 0), i8 %7)
  %n14 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @14, i32 0, i32 0))
  %8 = extractvalue [11 x i8] %load, 7
  %n15 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @15, i32 0, i32 0), i8 %8)
  %n16 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @16, i32 0, i32 0))
  %9 = extractvalue [11 x i8] %load, 8
  %n17 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @17, i32 0, i32 0), i8 %9)
  %n18 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @18, i32 0, i32 0))
  %10 = extractvalue [11 x i8] %load, 9
  %n19 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @19, i32 0, i32 0), i8 %10)
  %n20 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @20, i32 0, i32 0))
  %11 = extractvalue [11 x i8] %load, 10
  %n21 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @21, i32 0, i32 0), i8 %11)
  %n22 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @22, i32 0, i32 0))
  %n23 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @23, i32 0, i32 0), i8 10)
  %ptrcast = bitcast [11 x i8]* %a to i8*
  %tmpptr = alloca i8*, align 8
  store volatile i8* %ptrcast, i8** %tmpptr, align 8
  %tmpexpr = load i8*, i8** %tmpptr, align 8
  %b = alloca i8*, align 8
  store volatile i8* %tmpexpr, i8** %b, align 8
  %load24 = load i8*, i8** %b, align 8
  %exprarg = load i8*, i8** %b, align 8
  %printf = call i32 (i8*, ...) @printf(i8* %exprarg)
  br label %ret

ret:                                              ; preds = %entry
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #0

attributes #0 = { argmemonly nofree nosync nounwind willreturn }
