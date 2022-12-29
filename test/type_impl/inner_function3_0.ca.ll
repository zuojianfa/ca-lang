; ModuleID = 'inner_function3_0.ca'
source_filename = "inner_function3_0.ca"

declare i32 @printf(i8*, ...)

define void @main() !dbg !4 {
entry:
  br i1 true, label %then0, label %cond1, !dbg !8

cond1:                                            ; preds = %entry
  br label %outbb, !dbg !8

then0:                                            ; preds = %entry
  br label %outbb, !dbg !9

outbb:                                            ; preds = %then0, %cond1
  br label %ret, !dbg !11

ret:                                              ; preds = %outbb
  ret void, !dbg !11
}

define void @"_CA$FL2F4main5hello"() !dbg !12 {
entry:
  br label %ret, !dbg !13

ret:                                              ; preds = %entry
  ret void, !dbg !13
}

!llvm.module.flags = !{!0}
!llvm.dbg.cu = !{!1}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = distinct !DICompileUnit(language: DW_LANG_C99, file: !2, producer: "ca compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !3)
!2 = !DIFile(filename: "inner_function3_0.ca", directory: ".")
!3 = !{}
!4 = distinct !DISubprogram(name: "main", scope: !2, file: !2, line: 1, type: !5, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !1, retainedNodes: !3)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!8 = !DILocation(line: 2, column: 13, scope: !4)
!9 = !DILocation(line: 4, column: 13, scope: !10)
!10 = distinct !DILexicalBlock(scope: !4, file: !2, line: 4, column: 13)
!11 = !DILocation(line: 7, column: 1, scope: !4)
!12 = distinct !DISubprogram(name: "_CA$FL2F4main5hello", scope: !2, file: !2, line: 4, type: !5, scopeLine: 4, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !1, retainedNodes: !3)
!13 = !DILocation(line: 5, column: 2, scope: !12)
