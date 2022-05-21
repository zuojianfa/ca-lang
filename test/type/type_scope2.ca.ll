; ModuleID = 'type_scope2.ca'
source_filename = "type_scope2.ca"

%AA.0 = type { double }
%AA = type { i32, i1 }

declare i32 @printf(i8*, ...)

define void @main() !dbg !4 {
entry:
  %b = alloca i1, align 1
  %a1 = alloca %AA.0, align 8
  %0 = alloca %AA.0, align 8
  %c = alloca i32, align 4
  %a = alloca %AA, align 8
  %1 = alloca %AA, align 8
  %2 = getelementptr %AA, %AA* %1, i32 0, i32 0, !dbg !24
  store volatile i32 3231, i32* %2, align 4, !dbg !24
  %3 = getelementptr %AA, %AA* %1, i32 0, i32 1, !dbg !24
  store volatile i1 true, i1* %3, align 1, !dbg !24
  %4 = bitcast %AA* %a to i8*, !dbg !24
  %5 = bitcast %AA* %1 to i8*, !dbg !24
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %4, i8* align 8 %5, i64 8, i1 false), !dbg !24
  call void @llvm.dbg.declare(metadata %AA* %a, metadata !9, metadata !DIExpression()), !dbg !25
  store volatile i32 1, i32* %c, align 4, !dbg !26
  call void @llvm.dbg.declare(metadata i32* %c, metadata !16, metadata !DIExpression()), !dbg !27
  %6 = getelementptr %AA.0, %AA.0* %0, i32 0, i32 0, !dbg !28
  store volatile double 1.011100e+02, double* %6, align 8, !dbg !28
  %7 = bitcast %AA.0* %a1 to i8*, !dbg !28
  %8 = bitcast %AA.0* %0 to i8*, !dbg !28
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %7, i8* align 8 %8, i64 8, i1 false), !dbg !28
  call void @llvm.dbg.declare(metadata %AA.0* %a1, metadata !17, metadata !DIExpression()), !dbg !29
  store volatile i1 true, i1* %b, align 1, !dbg !30
  call void @llvm.dbg.declare(metadata i1* %b, metadata !23, metadata !DIExpression()), !dbg !31
  br label %ret, !dbg !32

ret:                                              ; preds = %entry
  ret void, !dbg !32
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
!2 = !DIFile(filename: "type_scope2.ca", directory: ".")
!3 = !{}
!4 = distinct !DISubprogram(name: "main", scope: !2, file: !2, line: 6, type: !5, scopeLine: 6, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !1, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!8 = !{!9, !16, !17, !23}
!9 = !DILocalVariable(name: "a", scope: !4, file: !2, line: 7, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "AA", scope: !4, file: !2, line: 5, size: 64, elements: !11)
!11 = !{!12, !14}
!12 = !DIDerivedType(tag: DW_TAG_member, name: "f1", scope: !10, file: !2, line: 5, baseType: !13, size: 32)
!13 = !DIBasicType(name: "i32", size: 32, encoding: DW_ATE_signed)
!14 = !DIDerivedType(tag: DW_TAG_member, name: "f2", scope: !10, file: !2, line: 5, baseType: !15, size: 8, offset: 32)
!15 = !DIBasicType(name: "bool", size: 8, encoding: DW_ATE_boolean)
!16 = !DILocalVariable(name: "c", scope: !4, file: !2, line: 8, type: !13)
!17 = !DILocalVariable(name: "a", scope: !18, file: !2, line: 13, type: !19)
!18 = distinct !DILexicalBlock(scope: !4, file: !2, line: 12, column: 1)
!19 = !DICompositeType(tag: DW_TAG_structure_type, name: "AA", scope: !4, file: !2, line: 5, size: 64, elements: !20)
!20 = !{!21}
!21 = !DIDerivedType(tag: DW_TAG_member, name: "f1", scope: !19, file: !2, line: 5, baseType: !22, size: 64)
!22 = !DIBasicType(name: "f64", size: 64, encoding: DW_ATE_float)
!23 = !DILocalVariable(name: "b", scope: !18, file: !2, line: 14, type: !15)
!24 = !DILocation(line: 7, column: 35, scope: !4)
!25 = !DILocation(line: 7, scope: !4)
!26 = !DILocation(line: 8, column: 14, scope: !4)
!27 = !DILocation(line: 8, scope: !4)
!28 = !DILocation(line: 13, column: 23, scope: !18)
!29 = !DILocation(line: 13, scope: !18)
!30 = !DILocation(line: 14, column: 14, scope: !18)
!31 = !DILocation(line: 14, scope: !18)
!32 = !DILocation(line: 16, column: 1, scope: !4)
