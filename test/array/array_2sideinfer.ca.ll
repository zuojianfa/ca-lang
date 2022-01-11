; ModuleID = 'array_2sideinfer.ca'
source_filename = "array_2sideinfer.ca"

@constarray = internal constant [3 x i32] [i32 1, i32 2, i32 3], align 4
@constarray.1 = internal constant [3 x [2 x i32]] [[2 x i32] [i32 1, i32 2], [2 x i32] [i32 3, i32 4], [2 x i32] [i32 5, i32 6]], align 4
@constarray.2 = internal constant [2 x [3 x i32]] [[3 x i32] [i32 1, i32 2, i32 3], [3 x i32] [i32 4, i32 5, i32 6]], align 4
@constarray.3 = internal constant [2 x double] [double 1.100000e+00, double 2.200000e+00], align 4
@constarray.4 = internal constant [3 x double] [double 1.100000e+00, double 2.200000e+00, double 3.300000e+00], align 4
@constarray.5 = internal constant [2 x [3 x [2 x i32]]] [[3 x [2 x i32]] [[2 x i32] [i32 1, i32 2], [2 x i32] [i32 3, i32 4], [2 x i32] [i32 5, i32 6]], [3 x [2 x i32]] [[2 x i32] [i32 7, i32 8], [2 x i32] [i32 9, i32 10], [2 x i32] [i32 11, i32 12]]], align 4

declare i32 @printf(i8*, ...)

define void @main() {
entry:
  %a = alloca [3 x i32], align 4
  %0 = bitcast [3 x i32]* %a to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %0, i8* align 4 bitcast ([3 x i32]* @constarray to i8*), i64 12, i1 false)
  %b = alloca [3 x [2 x i32]], align 4
  %1 = bitcast [3 x [2 x i32]]* %b to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %1, i8* align 4 bitcast ([3 x [2 x i32]]* @constarray.1 to i8*), i64 24, i1 false)
  %c = alloca [2 x [3 x i32]], align 4
  %2 = bitcast [2 x [3 x i32]]* %c to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %2, i8* align 4 bitcast ([2 x [3 x i32]]* @constarray.2 to i8*), i64 24, i1 false)
  %d = alloca [2 x double], align 8
  %3 = bitcast [2 x double]* %d to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %3, i8* align 8 bitcast ([2 x double]* @constarray.3 to i8*), i64 16, i1 false)
  %e = alloca [3 x double], align 8
  %4 = bitcast [3 x double]* %e to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %4, i8* align 8 bitcast ([3 x double]* @constarray.4 to i8*), i64 24, i1 false)
  %f = alloca [2 x [3 x [2 x i32]]], align 4
  %5 = bitcast [2 x [3 x [2 x i32]]]* %f to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %5, i8* align 4 bitcast ([2 x [3 x [2 x i32]]]* @constarray.5 to i8*), i64 48, i1 false)
  br label %ret

ret:                                              ; preds = %entry
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #0

attributes #0 = { argmemonly nofree nosync nounwind willreturn }
