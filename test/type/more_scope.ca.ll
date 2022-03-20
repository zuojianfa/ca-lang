; ModuleID = 'more_scope.ca'
source_filename = "more_scope.ca"

%AA = type { double }
%AA.0 = type { i1 }
%AA.1 = type { i32, i32 }
%AA.2 = type { [3 x i32] }

@a = internal global double 3.132300e+00, align 4
@0 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@1 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@2 = private unnamed_addr constant [3 x i8] c"%s\00", align 1

declare i32 @printf(i8*, ...)

define i32 @func1(i32 %a, i32 %b) !dbg !8 {
entry:
  %a1 = alloca i32, align 4
  store volatile i32 %a, i32* %a1, align 4
  call void @llvm.dbg.declare(metadata i32* %a1, metadata !13, metadata !DIExpression()), !dbg !17
  %b2 = alloca i32, align 4
  store volatile i32 %b, i32* %b2, align 4
  call void @llvm.dbg.declare(metadata i32* %b2, metadata !15, metadata !DIExpression()), !dbg !17
  %retslot = alloca i32, align 4
  %v1 = load i32, i32* %a1, align 4, !dbg !18
  %v2 = load i32, i32* %b2, align 4, !dbg !18
  %add = add i32 %v1, %v2, !dbg !18
  %c = alloca i32, align 4, !dbg !18
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

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

define void @main() !dbg !22 {
entry:
  %0 = alloca %AA, align 8, !dbg !53
  %1 = getelementptr %AA, %AA* %0, i32 0, i32 0, !dbg !53
  store volatile double 3.232300e+00, double* %1, align 8, !dbg !53
  %a = alloca %AA, align 8, !dbg !53
  %2 = bitcast %AA* %a to i8*, !dbg !53
  %3 = bitcast %AA* %0 to i8*, !dbg !53
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %2, i8* align 8 %3, i64 8, i1 false), !dbg !53
  call void @llvm.dbg.declare(metadata %AA* %a, metadata !26, metadata !DIExpression()), !dbg !54
  %4 = alloca %AA.0, align 8, !dbg !55
  %5 = getelementptr %AA.0, %AA.0* %4, i32 0, i32 0, !dbg !55
  store volatile i1 true, i1* %5, align 1, !dbg !55
  %a1 = alloca %AA.0, align 8, !dbg !55
  %6 = bitcast %AA.0* %a1 to i8*, !dbg !55
  %7 = bitcast %AA.0* %4 to i8*, !dbg !55
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %6, i8* align 8 %7, i64 1, i1 false), !dbg !55
  call void @llvm.dbg.declare(metadata %AA.0* %a1, metadata !30, metadata !DIExpression()), !dbg !56
  %8 = getelementptr inbounds %AA.0, %AA.0* %a1, i32 0, i32 0, !dbg !57
  store volatile i1 false, i1* %8, align 1, !dbg !57
  call void @llvm.dbg.declare(metadata i1* %8, metadata !36, metadata !DIExpression()), !dbg !58
  %9 = alloca %AA.1, align 8, !dbg !59
  %10 = getelementptr %AA.1, %AA.1* %9, i32 0, i32 0, !dbg !59
  store volatile i32 291, i32* %10, align 4, !dbg !59
  %11 = getelementptr %AA.1, %AA.1* %9, i32 0, i32 1, !dbg !59
  store volatile i32 323, i32* %11, align 4, !dbg !59
  %a2 = alloca %AA.1, align 8, !dbg !59
  %12 = bitcast %AA.1* %a2 to i8*, !dbg !59
  %13 = bitcast %AA.1* %9 to i8*, !dbg !59
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %12, i8* align 8 %13, i64 8, i1 false), !dbg !59
  call void @llvm.dbg.declare(metadata %AA.1* %a2, metadata !37, metadata !DIExpression()), !dbg !60
  %14 = getelementptr inbounds %AA.1, %AA.1* %a2, i32 0, i32 1, !dbg !61
  %v1 = load i32, i32* %14, align 4, !dbg !61
  %15 = getelementptr inbounds %AA.1, %AA.1* %a2, i32 0, i32 0, !dbg !61
  %v2 = load i32, i32* %15, align 4, !dbg !61
  %add = add i32 %v1, %v2, !dbg !61
  %b = alloca i32, align 4, !dbg !61
  store volatile i32 %add, i32* %b, align 4, !dbg !61
  call void @llvm.dbg.declare(metadata i32* %b, metadata !43, metadata !DIExpression()), !dbg !62
  %16 = getelementptr inbounds %AA.1, %AA.1* %a2, i32 0, i32 0, !dbg !63
  %exprarg = load i32, i32* %16, align 4, !dbg !63
  %17 = getelementptr inbounds %AA.1, %AA.1* %a2, i32 0, i32 1, !dbg !63
  %exprarg3 = load i32, i32* %17, align 4, !dbg !63
  %func1 = call i32 @func1(i32 %exprarg, i32 %exprarg3), !dbg !63
  %calltmp = alloca i32, align 4, !dbg !63
  store volatile i32 %func1, i32* %calltmp, align 4, !dbg !63
  %tmpexpr = load i32, i32* %calltmp, align 4, !dbg !63
  %c = alloca i32, align 4, !dbg !63
  store volatile i32 %tmpexpr, i32* %c, align 4, !dbg !63
  call void @llvm.dbg.declare(metadata i32* %c, metadata !44, metadata !DIExpression()), !dbg !64
  %v14 = load i32, i32* %b, align 4, !dbg !65
  %v25 = load i32, i32* %c, align 4, !dbg !65
  %eq = icmp eq i32 %v14, %v25, !dbg !66
  br i1 %eq, label %thenbb, label %outbb, !dbg !66

thenbb:                                           ; preds = %entry
  %18 = alloca [3 x i32], align 4, !dbg !67
  %19 = getelementptr [3 x i32], [3 x i32]* %18, i32 0, i64 0, !dbg !67
  store volatile i32 1, i32* %19, align 4, !dbg !67
  %20 = getelementptr [3 x i32], [3 x i32]* %18, i32 0, i64 1, !dbg !67
  store volatile i32 2, i32* %20, align 4, !dbg !67
  %21 = getelementptr [3 x i32], [3 x i32]* %18, i32 0, i64 2, !dbg !67
  store volatile i32 6, i32* %21, align 4, !dbg !67
  %22 = alloca %AA.2, align 8, !dbg !67
  %23 = getelementptr %AA.2, %AA.2* %22, i32 0, i32 0, !dbg !67
  %24 = bitcast [3 x i32]* %23 to i8*, !dbg !67
  %25 = bitcast [3 x i32]* %18 to i8*, !dbg !67
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %24, i8* align 1 %25, i64 12, i1 false), !dbg !67
  %a6 = alloca %AA.2, align 8, !dbg !67
  %26 = bitcast %AA.2* %a6 to i8*, !dbg !67
  %27 = bitcast %AA.2* %22 to i8*, !dbg !67
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %26, i8* align 8 %27, i64 12, i1 false), !dbg !67
  call void @llvm.dbg.declare(metadata %AA.2* %a6, metadata !45, metadata !DIExpression()), !dbg !68
  %28 = getelementptr inbounds %AA.2, %AA.2* %a6, i32 0, i32 0, !dbg !67
  %29 = getelementptr inbounds [3 x i32], [3 x i32]* %28, i32 0, i32 2, !dbg !69
  %load = load i32, i32* %29, align 4, !dbg !69
  %n = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i32 %load), !dbg !69
  %n7 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @2, i32 0, i32 0), i8* getelementptr inbounds ([2 x i8], [2 x i8]* @1, i32 0, i32 0)), !dbg !70
  br label %outbb, !dbg !70

outbb:                                            ; preds = %thenbb, %entry
  br label %ret, !dbg !71

ret:                                              ; preds = %outbb
  ret void, !dbg !71
}

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
!25 = !{!26, !30, !36, !37, !43, !44, !45}
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
!36 = !DILocalVariable(name: "t:void", scope: !31, file: !2, line: 19, type: !35)
!37 = !DILocalVariable(name: "a", scope: !38, file: !2, line: 26, type: !39)
!38 = distinct !DILexicalBlock(scope: !31, file: !2, line: 24, column: 5)
!39 = !DICompositeType(tag: DW_TAG_structure_type, name: "AA", scope: !22, file: !2, line: 5, size: 64, elements: !40)
!40 = !{!41, !42}
!41 = !DIDerivedType(tag: DW_TAG_member, name: "f1", scope: !39, file: !2, line: 5, baseType: !14, size: 32)
!42 = !DIDerivedType(tag: DW_TAG_member, name: "f2", scope: !39, file: !2, line: 5, baseType: !14, size: 32, offset: 32)
!43 = !DILocalVariable(name: "b", scope: !38, file: !2, line: 27, type: !14)
!44 = !DILocalVariable(name: "c", scope: !38, file: !2, line: 28, type: !14)
!45 = !DILocalVariable(name: "a", scope: !46, file: !2, line: 31, type: !47)
!46 = distinct !DILexicalBlock(scope: !38, file: !2, line: 30, column: 27)
!47 = !DICompositeType(tag: DW_TAG_structure_type, name: "AA", scope: !22, file: !2, line: 5, size: 96, elements: !48)
!48 = !{!49}
!49 = !DIDerivedType(tag: DW_TAG_member, name: "dddd", scope: !47, file: !2, line: 5, baseType: !50, size: 96)
!50 = !DICompositeType(tag: DW_TAG_array_type, baseType: !14, size: 96, elements: !51)
!51 = !{!52}
!52 = !DISubrange(count: 3, lowerBound: 0)
!53 = !DILocation(line: 13, column: 27, scope: !22)
!54 = !DILocation(line: 13, scope: !22)
!55 = !DILocation(line: 18, column: 17, scope: !31)
!56 = !DILocation(line: 18, scope: !31)
!57 = !DILocation(line: 19, column: 13, scope: !31)
!58 = !DILocation(line: 19, scope: !31)
!59 = !DILocation(line: 26, column: 34, scope: !38)
!60 = !DILocation(line: 26, scope: !38)
!61 = !DILocation(line: 27, column: 24, scope: !38)
!62 = !DILocation(line: 27, scope: !38)
!63 = !DILocation(line: 28, column: 29, scope: !38)
!64 = !DILocation(line: 28, scope: !38)
!65 = !DILocation(line: 29, column: 11, scope: !38)
!66 = !DILocation(line: 29, column: 16, scope: !38)
!67 = !DILocation(line: 31, column: 26, scope: !46)
!68 = !DILocation(line: 31, scope: !46)
!69 = !DILocation(line: 32, column: 17, scope: !46)
!70 = !DILocation(line: 32, column: 29, scope: !46)
!71 = !DILocation(line: 36, column: 1, scope: !22)
