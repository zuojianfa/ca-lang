; ModuleID = 'struct_use3.ca'
source_filename = "struct_use3.ca"

%A7 = type { %A7*, %BB }
%BB = type { %B1, i32 }
%B1 = type { %B2 }
%B2 = type { %B2*, %A7* }
%A6 = type { %A5, %A6* }
%A5 = type { %A5*, %A4 }
%A4 = type { %A3, %A4* }
%A3 = type { %A3*, %A2 }
%A2 = type { %A1, %A2* }
%A1 = type { %A1*, i32 }
%AA = type { %AA* }

@0 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@1 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@2 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@3 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@4 = private unnamed_addr constant [3 x i8] c"%c\00", align 1

declare i32 @printf(i8*, ...)

define void @main() {
entry:
  %v7 = alloca %A7, align 8
  %v6 = alloca %A6, align 8
  %v5 = alloca %A5, align 8
  %v4 = alloca %A4, align 8
  %v3 = alloca %A3, align 8
  %v2 = alloca %A2, align 8
  %v1 = alloca %A1, align 8
  %v0 = alloca %AA, align 8
  %pv0 = alloca %AA*, align 8
  %0 = bitcast %AA** %pv0 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %0, i8 0, i64 8, i1 false)
  %1 = bitcast %AA* %v0 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %1, i8 0, i64 8, i1 false)
  %2 = bitcast %A1* %v1 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %2, i8 0, i64 16, i1 false)
  %3 = bitcast %A2* %v2 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %3, i8 0, i64 24, i1 false)
  %4 = bitcast %A3* %v3 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %4, i8 0, i64 32, i1 false)
  %5 = bitcast %A4* %v4 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %5, i8 0, i64 40, i1 false)
  %6 = bitcast %A5* %v5 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %6, i8 0, i64 48, i1 false)
  %7 = bitcast %A6* %v6 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %7, i8 0, i64 56, i1 false)
  %8 = bitcast %A7* %v7 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %8, i8 0, i64 32, i1 false)
  %n = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 103)
  %n1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @1, i32 0, i32 0), i8 111)
  %n2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @2, i32 0, i32 0), i8 111)
  %n3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @3, i32 0, i32 0), i8 100)
  %n4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i8 10)
  br label %ret

ret:                                              ; preds = %entry
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #0

attributes #0 = { argmemonly nofree nosync nounwind willreturn writeonly }
