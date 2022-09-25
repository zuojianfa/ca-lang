; ModuleID = 'struct_one1.ca'
source_filename = "struct_one1.ca"

%A3 = type { double, i32 }
%A2 = type { i32, double }
%A1 = type { double }
%AA = type { i32 }

@0 = private unnamed_addr constant [3 x i8] c"AA\00", align 1
@1 = private unnamed_addr constant [6 x i8] c"%s { \00", align 1
@2 = private unnamed_addr constant [3 x i8] c"f1\00", align 1
@3 = private unnamed_addr constant [5 x i8] c"%s: \00", align 1
@4 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@5 = private unnamed_addr constant [3 x i8] c" }\00", align 1
@6 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@7 = private unnamed_addr constant [3 x i8] c"A1\00", align 1
@8 = private unnamed_addr constant [4 x i8] c"%lf\00", align 1
@9 = private unnamed_addr constant [3 x i8] c"A2\00", align 1
@10 = private unnamed_addr constant [3 x i8] c", \00", align 1
@11 = private unnamed_addr constant [3 x i8] c"f2\00", align 1
@12 = private unnamed_addr constant [3 x i8] c"A3\00", align 1

declare i32 @printf(i8*, ...)

define void @main() !dbg !4 {
entry:
  %a3 = alloca %A3, align 8
  %0 = alloca %A3, align 8
  %a2 = alloca %A2, align 8
  %1 = alloca %A2, align 8
  %a1 = alloca %A1, align 8
  %2 = alloca %A1, align 8
  %aa = alloca %AA, align 8
  %3 = alloca %AA, align 8
  %4 = getelementptr %AA, %AA* %3, i32 0, i32 0, !dbg !29
  store volatile i32 32, i32* %4, align 4, !dbg !29
  %5 = bitcast %AA* %aa to i8*, !dbg !29
  %6 = bitcast %AA* %3 to i8*, !dbg !29
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %5, i8* align 8 %6, i64 4, i1 false), !dbg !29
  call void @llvm.dbg.declare(metadata %AA* %aa, metadata !9, metadata !DIExpression()), !dbg !30
  %7 = getelementptr %A1, %A1* %2, i32 0, i32 0, !dbg !31
  store volatile double 4.430000e+01, double* %7, align 8, !dbg !31
  %8 = bitcast %A1* %a1 to i8*, !dbg !31
  %9 = bitcast %A1* %2 to i8*, !dbg !31
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %8, i8* align 8 %9, i64 8, i1 false), !dbg !31
  call void @llvm.dbg.declare(metadata %A1* %a1, metadata !14, metadata !DIExpression()), !dbg !32
  %10 = getelementptr %A2, %A2* %1, i32 0, i32 0, !dbg !33
  store volatile i32 32, i32* %10, align 4, !dbg !33
  %11 = getelementptr %A2, %A2* %1, i32 0, i32 1, !dbg !33
  store volatile double 4.430000e+01, double* %11, align 8, !dbg !33
  %12 = bitcast %A2* %a2 to i8*, !dbg !33
  %13 = bitcast %A2* %1 to i8*, !dbg !33
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %12, i8* align 8 %13, i64 16, i1 false), !dbg !33
  call void @llvm.dbg.declare(metadata %A2* %a2, metadata !19, metadata !DIExpression()), !dbg !34
  %14 = getelementptr %A3, %A3* %0, i32 0, i32 0, !dbg !35
  store volatile double 4.430000e+01, double* %14, align 8, !dbg !35
  %15 = getelementptr %A3, %A3* %0, i32 0, i32 1, !dbg !35
  store volatile i32 32, i32* %15, align 4, !dbg !35
  %16 = bitcast %A3* %a3 to i8*, !dbg !35
  %17 = bitcast %A3* %0 to i8*, !dbg !35
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %16, i8* align 8 %17, i64 16, i1 false), !dbg !35
  call void @llvm.dbg.declare(metadata %A3* %a3, metadata !24, metadata !DIExpression()), !dbg !36
  %load = load %AA, %AA* %aa, align 4, !dbg !37
  %n = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @1, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0)), !dbg !37
  %n1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @3, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @2, i32 0, i32 0)), !dbg !37
  %18 = extractvalue %AA %load, 0, !dbg !37
  %n2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32 %18), !dbg !37
  %n3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0)), !dbg !37
  %n4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @6, i32 0, i32 0), i8 10), !dbg !38
  %load5 = load %A1, %A1* %a1, align 8, !dbg !39
  %n6 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @1, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @7, i32 0, i32 0)), !dbg !39
  %n7 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @3, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @2, i32 0, i32 0)), !dbg !39
  %19 = extractvalue %A1 %load5, 0, !dbg !39
  %n8 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @8, i32 0, i32 0), double %19), !dbg !39
  %n9 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0)), !dbg !39
  %n10 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @6, i32 0, i32 0), i8 10), !dbg !40
  %load11 = load %A2, %A2* %a2, align 8, !dbg !41
  %n12 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @1, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @9, i32 0, i32 0)), !dbg !41
  %n13 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @3, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @2, i32 0, i32 0)), !dbg !41
  %20 = extractvalue %A2 %load11, 0, !dbg !41
  %n14 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32 %20), !dbg !41
  %n15 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @10, i32 0, i32 0)), !dbg !41
  %n16 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @3, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @11, i32 0, i32 0)), !dbg !41
  %21 = extractvalue %A2 %load11, 1, !dbg !41
  %n17 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @8, i32 0, i32 0), double %21), !dbg !41
  %n18 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0)), !dbg !41
  %n19 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @6, i32 0, i32 0), i8 10), !dbg !42
  %load20 = load %A3, %A3* %a3, align 8, !dbg !43
  %n21 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @1, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @12, i32 0, i32 0)), !dbg !43
  %n22 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @3, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @11, i32 0, i32 0)), !dbg !43
  %22 = extractvalue %A3 %load20, 0, !dbg !43
  %n23 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @8, i32 0, i32 0), double %22), !dbg !43
  %n24 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @10, i32 0, i32 0)), !dbg !43
  %n25 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @3, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @2, i32 0, i32 0)), !dbg !43
  %23 = extractvalue %A3 %load20, 1, !dbg !43
  %n26 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32 %23), !dbg !43
  %n27 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0)), !dbg !43
  %n28 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @6, i32 0, i32 0), i8 10), !dbg !44
  br label %ret, !dbg !45

ret:                                              ; preds = %entry
  ret void, !dbg !45
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
!2 = !DIFile(filename: "struct_one1.ca", directory: ".")
!3 = !{}
!4 = distinct !DISubprogram(name: "main", scope: !2, file: !2, line: 19, type: !5, scopeLine: 19, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !1, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!8 = !{!9, !14, !19, !24}
!9 = !DILocalVariable(name: "aa", scope: !4, file: !2, line: 20, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "AA", scope: !4, file: !2, line: 5, size: 32, elements: !11)
!11 = !{!12}
!12 = !DIDerivedType(tag: DW_TAG_member, name: "f1", scope: !10, file: !2, line: 5, baseType: !13, size: 32)
!13 = !DIBasicType(name: "i32", size: 32, encoding: DW_ATE_signed)
!14 = !DILocalVariable(name: "a1", scope: !4, file: !2, line: 21, type: !15)
!15 = !DICompositeType(tag: DW_TAG_structure_type, name: "A1", scope: !4, file: !2, line: 5, size: 64, elements: !16)
!16 = !{!17}
!17 = !DIDerivedType(tag: DW_TAG_member, name: "f1", scope: !15, file: !2, line: 5, baseType: !18, size: 64)
!18 = !DIBasicType(name: "f64", size: 64, encoding: DW_ATE_float)
!19 = !DILocalVariable(name: "a2", scope: !4, file: !2, line: 22, type: !20)
!20 = !DICompositeType(tag: DW_TAG_structure_type, name: "A2", scope: !4, file: !2, line: 5, size: 128, elements: !21)
!21 = !{!22, !23}
!22 = !DIDerivedType(tag: DW_TAG_member, name: "f1", scope: !20, file: !2, line: 5, baseType: !13, size: 32)
!23 = !DIDerivedType(tag: DW_TAG_member, name: "f2", scope: !20, file: !2, line: 5, baseType: !18, size: 64, offset: 64)
!24 = !DILocalVariable(name: "a3", scope: !4, file: !2, line: 23, type: !25)
!25 = !DICompositeType(tag: DW_TAG_structure_type, name: "A3", scope: !4, file: !2, line: 5, size: 128, elements: !26)
!26 = !{!27, !28}
!27 = !DIDerivedType(tag: DW_TAG_member, name: "f2", scope: !25, file: !2, line: 5, baseType: !18, size: 64)
!28 = !DIDerivedType(tag: DW_TAG_member, name: "f1", scope: !25, file: !2, line: 5, baseType: !13, size: 32, offset: 64)
!29 = !DILocation(line: 20, column: 24, scope: !4)
!30 = !DILocation(line: 20, scope: !4)
!31 = !DILocation(line: 21, column: 26, scope: !4)
!32 = !DILocation(line: 21, scope: !4)
!33 = !DILocation(line: 22, column: 34, scope: !4)
!34 = !DILocation(line: 22, scope: !4)
!35 = !DILocation(line: 23, column: 34, scope: !4)
!36 = !DILocation(line: 23, scope: !4)
!37 = !DILocation(line: 24, column: 13, scope: !4)
!38 = !DILocation(line: 24, column: 25, scope: !4)
!39 = !DILocation(line: 25, column: 13, scope: !4)
!40 = !DILocation(line: 25, column: 25, scope: !4)
!41 = !DILocation(line: 26, column: 13, scope: !4)
!42 = !DILocation(line: 26, column: 25, scope: !4)
!43 = !DILocation(line: 27, column: 13, scope: !4)
!44 = !DILocation(line: 27, column: 25, scope: !4)
!45 = !DILocation(line: 28, column: 1, scope: !4)
