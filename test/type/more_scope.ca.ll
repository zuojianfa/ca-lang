; ModuleID = 'more_scope.ca'
source_filename = "more_scope.ca"

%AA.2 = type { [3 x i32] }
%AA.1 = type { i32, i32 }
%AA.0 = type { i1 }
%AA = type { double }

@a = internal global double 3.132300e+00, align 4
@0 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@1 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@2 = private unnamed_addr constant [3 x i8] c"%s\00", align 1

declare i32 @printf(i8*, ...)

define i32 @func1(i32 %a, i32 %b) !dbg !8 {
entry:
  %c = alloca i32, align 4
  %retslot = alloca i32, align 4
  %a1 = alloca i32, align 4
  store volatile i32 %a, i32* %a1, align 4
  call void @llvm.dbg.declare(metadata i32* %a1, metadata !13, metadata !DIExpression()), !dbg !17
  %b2 = alloca i32, align 4
  store volatile i32 %b, i32* %b2, align 4
  call void @llvm.dbg.declare(metadata i32* %b2, metadata !15, metadata !DIExpression()), !dbg !17
  %v1 = load i32, i32* %a1, align 4, !dbg !18
  %v2 = load i32, i32* %b2, align 4, !dbg !18
  %add = add i32 %v1, %v2, !dbg !18
  store volatile i32 %add, i32* %c, align 4, !dbg !18
  call void @llvm.dbg.declare(metadata i32* %c, metadata !16, metadata !DIExpression()), !dbg !19
  %load = load i32, i32* %c, align 4, !dbg !18
  store i32 %load, i32* %retslot, align 4, !dbg !20
  br label %ret, !dbg !20

afterret:                                         ; No predecessors!
  br label %ret, !dbg !21

ret:                                              ; preds = %afterret, %entry
  %retret = load i32, i32* %retslot, align 4, !dbg !21
  ret i32 %retret, !dbg !21
}

define void @main() !dbg !22 {
entry:
  %a6 = alloca %AA.2, align 8
  %0 = alloca %AA.2, align 8
  %1 = alloca [3 x i32], align 4
  %c = alloca i32, align 4
  %calltmp = alloca i32, align 4
  %b = alloca i32, align 4
  %a2 = alloca %AA.1, align 8
  %2 = alloca %AA.1, align 8
  %a1 = alloca %AA.0, align 8
  %3 = alloca %AA.0, align 8
  %a = alloca %AA, align 8
  %4 = alloca %AA, align 8
  %5 = getelementptr %AA, %AA* %4, i32 0, i32 0, !dbg !52
  store volatile double 3.232300e+00, double* %5, align 8, !dbg !52
  %6 = bitcast %AA* %a to i8*, !dbg !52
  %7 = bitcast %AA* %4 to i8*, !dbg !52
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %6, i8* align 8 %7, i64 8, i1 false), !dbg !52
  call void @llvm.dbg.declare(metadata %AA* %a, metadata !26, metadata !DIExpression()), !dbg !53
  %8 = getelementptr %AA.0, %AA.0* %3, i32 0, i32 0, !dbg !54
  store volatile i1 true, i1* %8, align 1, !dbg !54
  %9 = bitcast %AA.0* %a1 to i8*, !dbg !54
  %10 = bitcast %AA.0* %3 to i8*, !dbg !54
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %9, i8* align 8 %10, i64 1, i1 false), !dbg !54
  call void @llvm.dbg.declare(metadata %AA.0* %a1, metadata !30, metadata !DIExpression()), !dbg !55
  %11 = getelementptr inbounds %AA.0, %AA.0* %a1, i32 0, i32 0, !dbg !56
  store volatile i1 false, i1* %11, align 1, !dbg !56
  %12 = getelementptr %AA.1, %AA.1* %2, i32 0, i32 0, !dbg !57
  store volatile i32 291, i32* %12, align 4, !dbg !57
  %13 = getelementptr %AA.1, %AA.1* %2, i32 0, i32 1, !dbg !57
  store volatile i32 323, i32* %13, align 4, !dbg !57
  %14 = bitcast %AA.1* %a2 to i8*, !dbg !57
  %15 = bitcast %AA.1* %2 to i8*, !dbg !57
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %14, i8* align 8 %15, i64 8, i1 false), !dbg !57
  call void @llvm.dbg.declare(metadata %AA.1* %a2, metadata !36, metadata !DIExpression()), !dbg !58
  %16 = getelementptr inbounds %AA.1, %AA.1* %a2, i32 0, i32 1, !dbg !59
  %v1 = load i32, i32* %16, align 4, !dbg !59
  %17 = getelementptr inbounds %AA.1, %AA.1* %a2, i32 0, i32 0, !dbg !59
  %v2 = load i32, i32* %17, align 4, !dbg !59
  %add = add i32 %v1, %v2, !dbg !59
  store volatile i32 %add, i32* %b, align 4, !dbg !59
  call void @llvm.dbg.declare(metadata i32* %b, metadata !42, metadata !DIExpression()), !dbg !60
  %18 = getelementptr inbounds %AA.1, %AA.1* %a2, i32 0, i32 0, !dbg !61
  %exprarg = load i32, i32* %18, align 4, !dbg !61
  %19 = getelementptr inbounds %AA.1, %AA.1* %a2, i32 0, i32 1, !dbg !61
  %exprarg3 = load i32, i32* %19, align 4, !dbg !61
  %func1 = call i32 @func1(i32 %exprarg, i32 %exprarg3), !dbg !61
  store volatile i32 %func1, i32* %calltmp, align 4, !dbg !61
  %tmpexpr = load i32, i32* %calltmp, align 4, !dbg !61
  store volatile i32 %tmpexpr, i32* %c, align 4, !dbg !61
  call void @llvm.dbg.declare(metadata i32* %c, metadata !43, metadata !DIExpression()), !dbg !62
  %v14 = load i32, i32* %b, align 4, !dbg !63
  %v25 = load i32, i32* %c, align 4, !dbg !63
  %eq = icmp eq i32 %v14, %v25, !dbg !64
  br i1 %eq, label %then0, label %cond1, !dbg !64

cond1:                                            ; preds = %entry
  br label %outbb, !dbg !64

then0:                                            ; preds = %entry
  %20 = getelementptr [3 x i32], [3 x i32]* %1, i32 0, i64 0, !dbg !65
  store volatile i32 1, i32* %20, align 4, !dbg !65
  %21 = getelementptr [3 x i32], [3 x i32]* %1, i32 0, i64 1, !dbg !65
  store volatile i32 2, i32* %21, align 4, !dbg !65
  %22 = getelementptr [3 x i32], [3 x i32]* %1, i32 0, i64 2, !dbg !65
  store volatile i32 6, i32* %22, align 4, !dbg !65
  %23 = getelementptr %AA.2, %AA.2* %0, i32 0, i32 0, !dbg !65
  %24 = bitcast [3 x i32]* %23 to i8*, !dbg !65
  %25 = bitcast [3 x i32]* %1 to i8*, !dbg !65
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %24, i8* align 1 %25, i64 12, i1 false), !dbg !65
  %26 = bitcast %AA.2* %a6 to i8*, !dbg !65
  %27 = bitcast %AA.2* %0 to i8*, !dbg !65
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %26, i8* align 8 %27, i64 12, i1 false), !dbg !65
  call void @llvm.dbg.declare(metadata %AA.2* %a6, metadata !44, metadata !DIExpression()), !dbg !66
  %28 = getelementptr inbounds %AA.2, %AA.2* %a6, i32 0, i32 0, !dbg !67
  %29 = getelementptr inbounds [3 x i32], [3 x i32]* %28, i32 0, i32 2, !dbg !67
  %load = load i32, i32* %29, align 4, !dbg !67
  %n = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i32 %load), !dbg !67
  %n7 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @2, i32 0, i32 0), i8* getelementptr inbounds ([2 x i8], [2 x i8]* @1, i32 0, i32 0)), !dbg !68
  br label %outbb, !dbg !68

outbb:                                            ; preds = %then0, %cond1
  br label %ret, !dbg !69

ret:                                              ; preds = %outbb
  ret void, !dbg !69
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #1

attributes #0 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }

!llvm.module.flags = !{!0}
!llvm.dbg.cu = !{!1}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = distinct !DICompileUnit(language: DW_LANG_C99, file: !2, producer: "ca compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !3, globals: !4)
!2 = !DIFile(filename: "more_scope.ca", directory: ".")
!3 = !{}
!4 = !{!5}
!5 = !DIGlobalVariableExpression(var: !6, expr: !DIExpression())
!6 = distinct !DIGlobalVariable(name: "a", scope: !2, file: !2, line: 5, type: !7, isLocal: false, isDefinition: true)
!7 = !DIBasicType(name: "f64", size: 64, encoding: DW_ATE_float)
!8 = distinct !DISubprogram(name: "func1", scope: !2, file: !2, line: 7, type: !9, scopeLine: 7, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !1, retainedNodes: !12)
!9 = !DISubroutineType(types: !10)
!10 = !{!11, !11, !11}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !{!13, !15, !16}
!13 = !DILocalVariable(name: "a", scope: !8, file: !2, line: 7, type: !14)
!14 = !DIBasicType(name: "i32", size: 32, encoding: DW_ATE_signed)
!15 = !DILocalVariable(name: "b", arg: 1, scope: !8, file: !2, line: 7, type: !14)
!16 = !DILocalVariable(name: "c", scope: !8, file: !2, line: 8, type: !14)
!17 = !DILocation(line: 7, scope: !8)
!18 = !DILocation(line: 8, column: 18, scope: !8)
!19 = !DILocation(line: 8, scope: !8)
!20 = !DILocation(line: 9, column: 13, scope: !8)
!21 = !DILocation(line: 10, column: 1, scope: !8)
!22 = distinct !DISubprogram(name: "main", scope: !2, file: !2, line: 12, type: !23, scopeLine: 12, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !1, retainedNodes: !25)
!23 = !DISubroutineType(types: !24)
!24 = !{!11}
!25 = !{!26, !30, !36, !42, !43, !44}
!26 = !DILocalVariable(name: "a", scope: !22, file: !2, line: 13, type: !27)
!27 = !DICompositeType(tag: DW_TAG_structure_type, name: "AA", scope: !22, file: !2, line: 5, size: 64, elements: !28)
!28 = !{!29}
!29 = !DIDerivedType(tag: DW_TAG_member, name: "f1", scope: !27, file: !2, line: 5, baseType: !7, size: 64)
!30 = !DILocalVariable(name: "a", scope: !31, file: !2, line: 18, type: !32)
!31 = distinct !DILexicalBlock(scope: !22, file: !2, line: 17, column: 1)
!32 = !DICompositeType(tag: DW_TAG_structure_type, name: "AA", scope: !22, file: !2, line: 5, size: 8, elements: !33)
!33 = !{!34}
!34 = !DIDerivedType(tag: DW_TAG_member, name: "c", scope: !32, file: !2, line: 5, baseType: !35, size: 8)
!35 = !DIBasicType(name: "bool", size: 8, encoding: DW_ATE_boolean)
!36 = !DILocalVariable(name: "a", scope: !37, file: !2, line: 26, type: !38)
!37 = distinct !DILexicalBlock(scope: !31, file: !2, line: 24, column: 5)
!38 = !DICompositeType(tag: DW_TAG_structure_type, name: "AA", scope: !22, file: !2, line: 5, size: 64, elements: !39)
!39 = !{!40, !41}
!40 = !DIDerivedType(tag: DW_TAG_member, name: "f1", scope: !38, file: !2, line: 5, baseType: !14, size: 32)
!41 = !DIDerivedType(tag: DW_TAG_member, name: "f2", scope: !38, file: !2, line: 5, baseType: !14, size: 32, offset: 32)
!42 = !DILocalVariable(name: "b", scope: !37, file: !2, line: 27, type: !14)
!43 = !DILocalVariable(name: "c", scope: !37, file: !2, line: 28, type: !14)
!44 = !DILocalVariable(name: "a", scope: !45, file: !2, line: 31, type: !46)
!45 = distinct !DILexicalBlock(scope: !37, file: !2, line: 30, column: 27)
!46 = !DICompositeType(tag: DW_TAG_structure_type, name: "AA", scope: !22, file: !2, line: 5, size: 96, elements: !47)
!47 = !{!48}
!48 = !DIDerivedType(tag: DW_TAG_member, name: "dddd", scope: !46, file: !2, line: 5, baseType: !49, size: 96)
!49 = !DICompositeType(tag: DW_TAG_array_type, baseType: !14, size: 96, elements: !50)
!50 = !{!51}
!51 = !DISubrange(count: 3, lowerBound: 0)
!52 = !DILocation(line: 13, column: 27, scope: !22)
!53 = !DILocation(line: 13, scope: !22)
!54 = !DILocation(line: 18, column: 17, scope: !31)
!55 = !DILocation(line: 18, scope: !31)
!56 = !DILocation(line: 19, column: 13, scope: !31)
!57 = !DILocation(line: 26, column: 34, scope: !37)
!58 = !DILocation(line: 26, scope: !37)
!59 = !DILocation(line: 27, column: 24, scope: !37)
!60 = !DILocation(line: 27, scope: !37)
!61 = !DILocation(line: 28, column: 29, scope: !37)
!62 = !DILocation(line: 28, scope: !37)
!63 = !DILocation(line: 29, column: 11, scope: !37)
!64 = !DILocation(line: 29, column: 16, scope: !37)
!65 = !DILocation(line: 31, column: 26, scope: !45)
!66 = !DILocation(line: 31, scope: !45)
!67 = !DILocation(line: 32, column: 17, scope: !45)
!68 = !DILocation(line: 32, column: 29, scope: !45)
!69 = !DILocation(line: 36, column: 1, scope: !22)
