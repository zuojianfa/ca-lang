; ModuleID = 'struct_array_s1.ca'
source_filename = "struct_array_s1.ca"

%AA = type { [3 x i32] }

declare i32 @printf(i8*, ...)

define void @main() !dbg !4 {
entry:
  %0 = alloca [3 x i32], align 4, !dbg !17
  %1 = getelementptr [3 x i32], [3 x i32]* %0, i32 0, i64 0, !dbg !17
  store volatile i32 1, i32* %1, align 4, !dbg !17
  %2 = getelementptr [3 x i32], [3 x i32]* %0, i32 0, i64 1, !dbg !17
  store volatile i32 2, i32* %2, align 4, !dbg !17
  %3 = getelementptr [3 x i32], [3 x i32]* %0, i32 0, i64 2, !dbg !17
  store volatile i32 3, i32* %3, align 4, !dbg !17
  %4 = alloca %AA, align 8, !dbg !17
  %5 = getelementptr %AA, %AA* %4, i32 0, i32 0, !dbg !17
  %6 = bitcast [3 x i32]* %5 to i8*, !dbg !17
  %7 = bitcast [3 x i32]* %0 to i8*, !dbg !17
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %6, i8* align 1 %7, i64 12, i1 false), !dbg !17
  %a = alloca %AA, align 8, !dbg !17
  %8 = bitcast %AA* %a to i8*, !dbg !17
  %9 = bitcast %AA* %4 to i8*, !dbg !17
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %8, i8* align 8 %9, i64 12, i1 false), !dbg !17
  call void @llvm.dbg.declare(metadata %AA* %a, metadata !9, metadata !DIExpression()), !dbg !18
  br label %ret, !dbg !19

ret:                                              ; preds = %entry
  ret void, !dbg !19
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
!2 = !DIFile(filename: "struct_array_s1.ca", directory: ".")
!3 = !{}
!4 = distinct !DISubprogram(name: "main", scope: !2, file: !2, line: 5, type: !5, scopeLine: 5, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !1, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!8 = !{!9}
!9 = !DILocalVariable(name: "a", scope: !4, file: !2, line: 6, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "AA", scope: !4, file: !2, line: 5, size: 96, elements: !11)
!11 = !{!12}
!12 = !DIDerivedType(tag: DW_TAG_member, name: "f1", scope: !10, file: !2, line: 5, baseType: !13, size: 96)
!13 = !DICompositeType(tag: DW_TAG_array_type, baseType: !14, size: 96, elements: !15)
!14 = !DIBasicType(name: "i32", size: 32, encoding: DW_ATE_signed)
!15 = !{!16}
!16 = !DISubrange(count: 3, lowerBound: 0)
!17 = !DILocation(line: 6, column: 25, scope: !4)
!18 = !DILocation(line: 6, scope: !4)
!19 = !DILocation(line: 8, column: 1, scope: !4)
