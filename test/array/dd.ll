; ModuleID = 'array_simple.ca'
source_filename = "array_simple.ca"

declare i32 @printf(i8*, ...)

define void @main() !dbg !4 {
entry:
  %0 = alloca [1 x i32], align 4, !dbg !10
  %1 = alloca i32, align 4
  %a = alloca [1 x i32], align 4, !dbg !10

  %2 = bitcast [1 x i32]* %a to i8*, !dbg !10
  %3 = bitcast [1 x i32]* %0 to i8*, !dbg !10
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %2, i8* align 4 %3, i64 4, i1 false), !dbg !10
  call void @llvm.dbg.declare(metadata [1 x i32]* %a, metadata !9, metadata !DIExpression()), !dbg !11
  br label %ret, !dbg !12

ret:                                              ; preds = %entry
  ret void, !dbg !12
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #0

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { argmemonly nofree nosync nounwind willreturn }
attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }

!llvm.module.flags = !{!0}
!llvm.dbg.cu = !{!1}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = distinct !DICompileUnit(language: DW_LANG_C99, file: !2, producer: "ca compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !3)
!2 = !DIFile(filename: "array_simple.ca", directory: ".")
!3 = !{}
!4 = distinct !DISubprogram(name: "main", scope: !2, file: !2, line: 1, type: !5, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !1, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!8 = !{!9}
!10 = !DILocation(line: 2, column: 25, scope: !4)
!11 = !DILocation(line: 2, scope: !4)
!12 = !DILocation(line: 4, column: 1, scope: !4)

!9 = !DILocalVariable(name: "a", scope: !4, file: !2, line: 2, type: !13)
!13 = !DICompositeType(tag: DW_TAG_array_type, baseType: !7, size: 32, elements: !14)
!14 = !{!15}
!15 = !DISubrange(count: 1)

