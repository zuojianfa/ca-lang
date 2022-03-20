; ModuleID = 'type_scope3.ca'
source_filename = "type_scope3.ca"

declare i32 @printf(i8*, ...)

define void @main() !dbg !4 {
entry:
  %a = alloca double, align 8, !dbg !14
  store volatile double 3.140000e+00, double* %a, align 8, !dbg !14
  call void @llvm.dbg.declare(metadata double* %a, metadata !9, metadata !DIExpression()), !dbg !15
  %a1 = alloca i32, align 4, !dbg !16
  store volatile i32 4, i32* %a1, align 4, !dbg !16
  call void @llvm.dbg.declare(metadata i32* %a1, metadata !11, metadata !DIExpression()), !dbg !17
  br label %ret, !dbg !18

ret:                                              ; preds = %entry
  ret void, !dbg !18
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

attributes #0 = { nofree nosync nounwind readnone speculatable willreturn }

!llvm.module.flags = !{!0}
!llvm.dbg.cu = !{!1}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = distinct !DICompileUnit(language: DW_LANG_C99, file: !2, producer: "ca compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !3)
!2 = !DIFile(filename: "type_scope3.ca", directory: ".")
!3 = !{}
!4 = distinct !DISubprogram(name: "main", scope: !2, file: !2, line: 1, type: !5, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !1, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!8 = !{!9, !11}
!9 = !DILocalVariable(name: "a", scope: !4, file: !2, line: 2, type: !10)
!10 = !DIBasicType(name: "f64", size: 64, encoding: DW_ATE_float)
!11 = !DILocalVariable(name: "a", scope: !12, file: !2, line: 4, type: !13)
!12 = distinct !DILexicalBlock(scope: !4, file: !2, line: 4, column: 10)
!13 = !DIBasicType(name: "i32", size: 32, encoding: DW_ATE_signed)
!14 = !DILocation(line: 2, column: 17, scope: !4)
!15 = !DILocation(line: 2, scope: !4)
!16 = !DILocation(line: 4, column: 11, scope: !12)
!17 = !DILocation(line: 4, scope: !12)
!18 = !DILocation(line: 6, column: 1, scope: !4)
