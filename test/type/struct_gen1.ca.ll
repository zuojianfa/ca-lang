; ModuleID = 'struct_gen1.ca'
source_filename = "struct_gen1.ca"

%AA.0 = type { %AA.0*, i32 }
%AA = type { %AA* }

declare i32 @printf(i8*, ...)

define void @main() {
entry:
  %bb1 = alloca %AA.0, align 8
  %bb = alloca %AA, align 8
  %aa = alloca %AA, align 8
  %0 = bitcast %AA* %aa to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %0, i8 0, i64 8, i1 false)
  %1 = bitcast %AA* %bb to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %1, i8 0, i64 8, i1 false)
  %2 = bitcast %AA.0* %bb1 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %2, i8 0, i64 16, i1 false)
  br label %ret

ret:                                              ; preds = %entry
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #0

attributes #0 = { argmemonly nofree nosync nounwind willreturn writeonly }
