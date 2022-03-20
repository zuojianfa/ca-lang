; ModuleID = 'scope1.c'
source_filename = "scope1.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.AA = type { double, i8 }
%struct.AA.0 = type { i8 }
%struct.AA.1 = type { i32 }

@__const.main.a = private unnamed_addr constant %struct.AA { double 1.230000e+00, i8 67 }, align 8
@__const.main.a.1 = private unnamed_addr constant %struct.AA.0 { i8 68 }, align 1
@__const.main.a.2 = private unnamed_addr constant %struct.AA.1 { i32 12 }, align 4

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 !dbg !7 {
  %1 = alloca i32, align 4
  %2 = alloca %struct.AA, align 8
  %3 = alloca %struct.AA.0, align 1
  %4 = alloca i32, align 4
  %5 = alloca %struct.AA.1, align 4
  %6 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  call void @llvm.dbg.declare(metadata %struct.AA* %2, metadata !11, metadata !DIExpression()), !dbg !18
  %7 = bitcast %struct.AA* %2 to i8*, !dbg !18
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %7, i8* align 8 bitcast (%struct.AA* @__const.main.a to i8*), i64 16, i1 false), !dbg !18
  call void @llvm.dbg.declare(metadata %struct.AA.0* %3, metadata !19, metadata !DIExpression()), !dbg !24
  %8 = bitcast %struct.AA.0* %3 to i8*, !dbg !24
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %8, i8* align 1 getelementptr inbounds (%struct.AA.0, %struct.AA.0* @__const.main.a.1, i32 0, i32 0), i64 1, i1 false), !dbg !24
  call void @llvm.dbg.declare(metadata i32* %4, metadata !25, metadata !DIExpression()), !dbg !26
  store i32 0, i32* %4, align 4, !dbg !26
  call void @llvm.dbg.declare(metadata %struct.AA.1* %5, metadata !27, metadata !DIExpression()), !dbg !32
  %9 = bitcast %struct.AA.1* %5 to i8*, !dbg !32
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %9, i8* align 4 bitcast (%struct.AA.1* @__const.main.a.2 to i8*), i64 4, i1 false), !dbg !32
  call void @llvm.dbg.declare(metadata i32* %6, metadata !33, metadata !DIExpression()), !dbg !34
  store i32 0, i32* %6, align 4, !dbg !34
  ret i32 0, !dbg !35
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #2

attributes #0 = { noinline nounwind optnone uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { argmemonly nofree nosync nounwind willreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 12.0.1", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "scope1.c", directory: "/home/rxia/git/compiler/ca/test/type")
!2 = !{}
!3 = !{i32 7, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"clang version 12.0.1"}
!7 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 6, type: !8, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DILocalVariable(name: "a", scope: !7, file: !1, line: 7, type: !12)
!12 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "AA", file: !1, line: 1, size: 128, elements: !13)
!13 = !{!14, !16}
!14 = !DIDerivedType(tag: DW_TAG_member, name: "a", scope: !12, file: !1, line: 2, baseType: !15, size: 64)
!15 = !DIBasicType(name: "double", size: 64, encoding: DW_ATE_float)
!16 = !DIDerivedType(tag: DW_TAG_member, name: "b", scope: !12, file: !1, line: 3, baseType: !17, size: 8, offset: 64)
!17 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!18 = !DILocation(line: 7, column: 13, scope: !7)
!19 = !DILocalVariable(name: "a", scope: !20, file: !1, line: 13, type: !21)
!20 = distinct !DILexicalBlock(scope: !7, file: !1, line: 8, column: 3)
!21 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "AA", scope: !7, file: !1, line: 9, size: 8, elements: !22)
!22 = !{!23}
!23 = !DIDerivedType(tag: DW_TAG_member, name: "b", scope: !21, file: !1, line: 10, baseType: !17, size: 8)
!24 = !DILocation(line: 13, column: 15, scope: !20)
!25 = !DILocalVariable(name: "i", scope: !20, file: !1, line: 14, type: !10)
!26 = !DILocation(line: 14, column: 9, scope: !20)
!27 = !DILocalVariable(name: "a", scope: !28, file: !1, line: 20, type: !29)
!28 = distinct !DILexicalBlock(scope: !20, file: !1, line: 15, column: 5)
!29 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "AA", scope: !7, file: !1, line: 16, size: 32, elements: !30)
!30 = !{!31}
!31 = !DIDerivedType(tag: DW_TAG_member, name: "a", scope: !29, file: !1, line: 17, baseType: !10, size: 32)
!32 = !DILocation(line: 20, column: 17, scope: !28)
!33 = !DILocalVariable(name: "b", scope: !28, file: !1, line: 21, type: !10)
!34 = !DILocation(line: 21, column: 11, scope: !28)
!35 = !DILocation(line: 24, column: 3, scope: !7)
