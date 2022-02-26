; ModuleID = 'struct_use2.ca'
source_filename = "struct_use2.ca"

%AA = type {}
%A1 = type { i32 }
%A2 = type { double* }
%A3 = type { [64 x float] }
%A4 = type { %A1 }
%A5 = type { %A1* }
%A6 = type { [32 x %A1] }
%A7 = type { %A2 }
%A8 = type { %A2* }
%A9 = type { [32 x %A2] }
%A10 = type { %A3 }
%A11 = type { %A3* }
%A12 = type { [32 x %A3] }
%A13 = type { %A6 }
%A14 = type { %A6* }
%A15 = type { [32 x %A6] }
%A16 = type { %A12 }
%A17 = type { %A12* }
%A18 = type { [32 x %A12] }
%A19 = type { %A18 }
%A20 = type { %A18* }
%A21 = type { [32 x %A18] }
%A24 = type { %A10, %A21 }
%A25 = type { %A10*, [3 x %A21]** }
%A26 = type { [3 x %A10], [4 x %A25*]* }
%A28 = type { [3 x %A10], [4 x %A25*]*, %A10, %A10*, %A25 }

declare i32 @printf(i8*, ...)

define void @main() {
entry:
  %v0 = alloca %AA, align 8
  %0 = bitcast %AA* %v0 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %0, i8 0, i64 0, i1 false)
  %v1 = alloca %A1, align 8
  %1 = bitcast %A1* %v1 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %1, i8 0, i64 4, i1 false)
  %v2 = alloca %A2, align 8
  %2 = bitcast %A2* %v2 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %2, i8 0, i64 8, i1 false)
  %v3 = alloca %A3, align 8
  %3 = bitcast %A3* %v3 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %3, i8 0, i64 256, i1 false)
  %v4 = alloca %A4, align 8
  %4 = bitcast %A4* %v4 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %4, i8 0, i64 4, i1 false)
  %v5 = alloca %A5, align 8
  %5 = bitcast %A5* %v5 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %5, i8 0, i64 8, i1 false)
  %v6 = alloca %A6, align 8
  %6 = bitcast %A6* %v6 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %6, i8 0, i64 128, i1 false)
  %v7 = alloca %A7, align 8
  %7 = bitcast %A7* %v7 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %7, i8 0, i64 8, i1 false)
  %v8 = alloca %A8, align 8
  %8 = bitcast %A8* %v8 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %8, i8 0, i64 8, i1 false)
  %v9 = alloca %A9, align 8
  %9 = bitcast %A9* %v9 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %9, i8 0, i64 256, i1 false)
  %v10 = alloca %A10, align 8
  %10 = bitcast %A10* %v10 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %10, i8 0, i64 256, i1 false)
  %v11 = alloca %A11, align 8
  %11 = bitcast %A11* %v11 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %11, i8 0, i64 8, i1 false)
  %v12 = alloca %A12, align 8
  %12 = bitcast %A12* %v12 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %12, i8 0, i64 8192, i1 false)
  %v13 = alloca %A13, align 8
  %13 = bitcast %A13* %v13 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %13, i8 0, i64 128, i1 false)
  %v14 = alloca %A14, align 8
  %14 = bitcast %A14* %v14 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %14, i8 0, i64 8, i1 false)
  %v15 = alloca %A15, align 8
  %15 = bitcast %A15* %v15 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %15, i8 0, i64 4096, i1 false)
  %v16 = alloca %A16, align 8
  %16 = bitcast %A16* %v16 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %16, i8 0, i64 8192, i1 false)
  %v17 = alloca %A17, align 8
  %17 = bitcast %A17* %v17 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %17, i8 0, i64 8, i1 false)
  %v18 = alloca %A18, align 8
  %18 = bitcast %A18* %v18 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %18, i8 0, i64 262144, i1 false)
  %v19 = alloca %A19, align 8
  %19 = bitcast %A19* %v19 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %19, i8 0, i64 262144, i1 false)
  %v20 = alloca %A20, align 8
  %20 = bitcast %A20* %v20 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %20, i8 0, i64 8, i1 false)
  %v21 = alloca %A21, align 8
  %21 = bitcast %A21* %v21 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %21, i8 0, i64 8388608, i1 false)
  %v22 = alloca %A21, align 8
  %22 = bitcast %A21* %v22 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %22, i8 0, i64 8388608, i1 false)
  %v23 = alloca %A21, align 8
  %23 = bitcast %A21* %v23 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %23, i8 0, i64 8388608, i1 false)
  %v24 = alloca %A24, align 8
  %24 = bitcast %A24* %v24 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %24, i8 0, i64 8388864, i1 false)
  %v25 = alloca %A25, align 8
  %25 = bitcast %A25* %v25 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %25, i8 0, i64 16, i1 false)
  %v26 = alloca %A26, align 8
  %26 = bitcast %A26* %v26 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %26, i8 0, i64 776, i1 false)
  %v27 = alloca [6 x %A26*]*, align 8
  %27 = bitcast [6 x %A26*]** %v27 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %27, i8 0, i64 8, i1 false)
  %v28 = alloca %A28, align 8
  %28 = bitcast %A28* %v28 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %28, i8 0, i64 1056, i1 false)
  br label %ret

ret:                                              ; preds = %entry
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #0

attributes #0 = { argmemonly nofree nosync nounwind willreturn writeonly }
