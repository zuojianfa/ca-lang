; ModuleID = 'type_scope1.ca'
source_filename = "type_scope1.ca"

%AA = type { i32, i1 }
%AA.0 = type { i1 }
%AA.1 = type { double }

@a = internal global i32 34, align 4
@0 = private unnamed_addr constant [3 x i8] c"AA\00", align 1
@1 = private unnamed_addr constant [6 x i8] c"%s { \00", align 1
@2 = private unnamed_addr constant [3 x i8] c"f2\00", align 1
@3 = private unnamed_addr constant [5 x i8] c"%s: \00", align 1
@4 = private unnamed_addr constant [4 x i8] c"%1d\00", align 1
@5 = private unnamed_addr constant [3 x i8] c" }\00", align 1
@6 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@7 = private unnamed_addr constant [3 x i8] c"%s\00", align 1
@8 = private unnamed_addr constant [3 x i8] c"AA\00", align 1
@9 = private unnamed_addr constant [6 x i8] c"%s { \00", align 1
@10 = private unnamed_addr constant [3 x i8] c"f1\00", align 1
@11 = private unnamed_addr constant [5 x i8] c"%s: \00", align 1
@12 = private unnamed_addr constant [4 x i8] c"%lf\00", align 1
@13 = private unnamed_addr constant [3 x i8] c" }\00", align 1
@14 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@15 = private unnamed_addr constant [3 x i8] c"%s\00", align 1
@16 = private unnamed_addr constant [3 x i8] c"AA\00", align 1
@17 = private unnamed_addr constant [6 x i8] c"%s { \00", align 1
@18 = private unnamed_addr constant [3 x i8] c"f1\00", align 1
@19 = private unnamed_addr constant [5 x i8] c"%s: \00", align 1
@20 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@21 = private unnamed_addr constant [3 x i8] c", \00", align 1
@22 = private unnamed_addr constant [3 x i8] c"f2\00", align 1
@23 = private unnamed_addr constant [5 x i8] c"%s: \00", align 1
@24 = private unnamed_addr constant [4 x i8] c"%1d\00", align 1
@25 = private unnamed_addr constant [3 x i8] c" }\00", align 1
@26 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@27 = private unnamed_addr constant [3 x i8] c"%s\00", align 1

declare i32 @printf(i8*, ...)

define void @func1() !dbg !8 {
entry:
  %0 = alloca %AA, align 8, !dbg !30
  %1 = getelementptr %AA, %AA* %0, i32 0, i32 0, !dbg !30
  store volatile i32 3231, i32* %1, align 4, !dbg !30
  %2 = getelementptr %AA, %AA* %0, i32 0, i32 1, !dbg !30
  store volatile i1 true, i1* %2, align 1, !dbg !30
  %a = alloca %AA, align 8, !dbg !30
  %3 = bitcast %AA* %a to i8*, !dbg !30
  %4 = bitcast %AA* %0 to i8*, !dbg !30
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %3, i8* align 8 %4, i64 8, i1 false), !dbg !30
  call void @llvm.dbg.declare(metadata %AA* %a, metadata !13, metadata !DIExpression()), !dbg !31
  %5 = alloca %AA.0, align 8, !dbg !32
  %6 = getelementptr %AA.0, %AA.0* %5, i32 0, i32 0, !dbg !32
  store volatile i1 true, i1* %6, align 1, !dbg !32
  %a1 = alloca %AA.0, align 8, !dbg !32
  %7 = bitcast %AA.0* %a1 to i8*, !dbg !32
  %8 = bitcast %AA.0* %5 to i8*, !dbg !32
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %7, i8* align 8 %8, i64 1, i1 false), !dbg !32
  call void @llvm.dbg.declare(metadata %AA.0* %a1, metadata !19, metadata !DIExpression()), !dbg !33
  %load = load %AA.0, %AA.0* %a1, align 1, !dbg !32
  %n = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @1, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0)), !dbg !32
  %n2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @3, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @2, i32 0, i32 0)), !dbg !32
  %9 = extractvalue %AA.0 %load, 0, !dbg !32
  %10 = zext i1 %9 to i32, !dbg !32
  %n3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @4, i32 0, i32 0), i32 %10), !dbg !32
  %n4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0)), !dbg !32
  %n5 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @7, i32 0, i32 0), i8* getelementptr inbounds ([2 x i8], [2 x i8]* @6, i32 0, i32 0)), !dbg !34
  %11 = alloca %AA.1, align 8, !dbg !35
  %12 = getelementptr %AA.1, %AA.1* %11, i32 0, i32 0, !dbg !35
  store volatile double 1.011100e+02, double* %12, align 8, !dbg !35
  %a6 = alloca %AA.1, align 8, !dbg !35
  %13 = bitcast %AA.1* %a6 to i8*, !dbg !35
  %14 = bitcast %AA.1* %11 to i8*, !dbg !35
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %13, i8* align 8 %14, i64 8, i1 false), !dbg !35
  call void @llvm.dbg.declare(metadata %AA.1* %a6, metadata !25, metadata !DIExpression()), !dbg !36
  %load7 = load %AA.1, %AA.1* %a6, align 8, !dbg !35
  %n8 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @9, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @8, i32 0, i32 0)), !dbg !35
  %n9 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @11, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @10, i32 0, i32 0)), !dbg !35
  %15 = extractvalue %AA.1 %load7, 0, !dbg !35
  %n10 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @12, i32 0, i32 0), double %15), !dbg !35
  %n11 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @13, i32 0, i32 0)), !dbg !35
  %n12 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @15, i32 0, i32 0), i8* getelementptr inbounds ([2 x i8], [2 x i8]* @14, i32 0, i32 0)), !dbg !37
  br label %ret, !dbg !38

ret:                                              ; preds = %entry
  ret void, !dbg !38
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #0

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

define void @main() !dbg !39 {
entry:
  call void @func1(), !dbg !42
  %0 = alloca %AA, align 8, !dbg !43
  %1 = getelementptr %AA, %AA* %0, i32 0, i32 0, !dbg !43
  store volatile i32 321, i32* %1, align 4, !dbg !43
  %2 = getelementptr %AA, %AA* %0, i32 0, i32 1, !dbg !43
  store volatile i1 true, i1* %2, align 1, !dbg !43
  %a = alloca %AA, align 8, !dbg !43
  %3 = bitcast %AA* %a to i8*, !dbg !43
  %4 = bitcast %AA* %0 to i8*, !dbg !43
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %3, i8* align 8 %4, i64 8, i1 false), !dbg !43
  call void @llvm.dbg.declare(metadata %AA* %a, metadata !41, metadata !DIExpression()), !dbg !44
  %load = load %AA, %AA* %a, align 4, !dbg !43
  %n = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @17, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @16, i32 0, i32 0)), !dbg !43
  %n1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @19, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @18, i32 0, i32 0)), !dbg !43
  %5 = extractvalue %AA %load, 0, !dbg !43
  %n2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @20, i32 0, i32 0), i32 %5), !dbg !43
  %n3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @21, i32 0, i32 0)), !dbg !43
  %n4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @23, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @22, i32 0, i32 0)), !dbg !43
  %6 = extractvalue %AA %load, 1, !dbg !43
  %7 = zext i1 %6 to i32, !dbg !43
  %n5 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @24, i32 0, i32 0), i32 %7), !dbg !43
  %n6 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @25, i32 0, i32 0)), !dbg !43
  %n7 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @27, i32 0, i32 0), i8* getelementptr inbounds ([2 x i8], [2 x i8]* @26, i32 0, i32 0)), !dbg !45
  br label %ret, !dbg !46

ret:                                              ; preds = %entry
  ret void, !dbg !46
}

attributes #0 = { argmemonly nofree nosync nounwind willreturn }
attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }

!llvm.module.flags = !{!0}
!llvm.dbg.cu = !{!1}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = distinct !DICompileUnit(language: DW_LANG_C99, file: !2, producer: "ca compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !3, globals: !4)
!2 = !DIFile(filename: "type_scope1.ca", directory: ".")
!3 = !{}
!4 = !{!5}
!5 = !DIGlobalVariableExpression(var: !6, expr: !DIExpression())
!6 = distinct !DIGlobalVariable(name: "a", scope: !2, file: !2, line: 6, type: !7, isLocal: false, isDefinition: true)
!7 = !DIBasicType(name: "i32", size: 32, encoding: DW_ATE_signed)
!8 = distinct !DISubprogram(name: "func1", scope: !2, file: !2, line: 8, type: !9, scopeLine: 8, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !1, retainedNodes: !12)
!9 = !DISubroutineType(types: !10)
!10 = !{!11}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !{!13, !19, !25}
!13 = !DILocalVariable(name: "a", scope: !8, file: !2, line: 9, type: !14)
!14 = !DICompositeType(tag: DW_TAG_structure_type, name: "AA", scope: !8, file: !2, line: 5, size: 64, elements: !15)
!15 = !{!16, !17}
!16 = !DIDerivedType(tag: DW_TAG_member, name: "f1", scope: !14, file: !2, line: 5, baseType: !7, size: 32)
!17 = !DIDerivedType(tag: DW_TAG_member, name: "f2", scope: !14, file: !2, line: 5, baseType: !18, size: 8, offset: 32)
!18 = !DIBasicType(name: "bool", size: 8, encoding: DW_ATE_boolean)
!19 = !DILocalVariable(name: "a", scope: !20, file: !2, line: 19, type: !22)
!20 = distinct !DILexicalBlock(scope: !21, file: !2, line: 17, column: 5)
!21 = distinct !DILexicalBlock(scope: !8, file: !2, line: 13, column: 1)
!22 = !DICompositeType(tag: DW_TAG_structure_type, name: "AA", scope: !8, file: !2, line: 5, size: 8, elements: !23)
!23 = !{!24}
!24 = !DIDerivedType(tag: DW_TAG_member, name: "f2", scope: !22, file: !2, line: 5, baseType: !18, size: 8)
!25 = !DILocalVariable(name: "a", scope: !21, file: !2, line: 23, type: !26)
!26 = !DICompositeType(tag: DW_TAG_structure_type, name: "AA", scope: !8, file: !2, line: 5, size: 64, elements: !27)
!27 = !{!28}
!28 = !DIDerivedType(tag: DW_TAG_member, name: "f1", scope: !26, file: !2, line: 5, baseType: !29, size: 64)
!29 = !DIBasicType(name: "f64", size: 64, encoding: DW_ATE_float)
!30 = !DILocation(line: 9, column: 35, scope: !8)
!31 = !DILocation(line: 9, scope: !8)
!32 = !DILocation(line: 19, column: 21, scope: !20)
!33 = !DILocation(line: 19, scope: !20)
!34 = !DILocation(line: 20, column: 24, scope: !20)
!35 = !DILocation(line: 23, column: 23, scope: !21)
!36 = !DILocation(line: 23, scope: !21)
!37 = !DILocation(line: 24, column: 20, scope: !21)
!38 = !DILocation(line: 26, column: 1, scope: !8)
!39 = distinct !DISubprogram(name: "main", scope: !2, file: !2, line: 28, type: !9, scopeLine: 28, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !1, retainedNodes: !40)
!40 = !{!41}
!41 = !DILocalVariable(name: "a", scope: !39, file: !2, line: 30, type: !14)
!42 = !DILocation(line: 29, column: 11, scope: !39)
!43 = !DILocation(line: 30, column: 33, scope: !39)
!44 = !DILocation(line: 30, scope: !39)
!45 = !DILocation(line: 31, column: 23, scope: !39)
!46 = !DILocation(line: 32, column: 1, scope: !39)
