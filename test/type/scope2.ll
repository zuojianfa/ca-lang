; ModuleID = 'scope2.c'
source_filename = "scope2.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.AA = type { i32, i8 }
%struct.AA.0 = type { double }

@__const.main.a = private unnamed_addr constant %struct.AA { i32 3231, i8 1 }, align 4
@__const.main.a.1 = private unnamed_addr constant %struct.AA.0 { double 1.011100e+02 }, align 8

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 !dbg !7 {
  %1 = alloca %struct.AA, align 4
  %2 = alloca %struct.AA.0, align 8
  call void @llvm.dbg.declare(metadata %struct.AA* %1, metadata !11, metadata !DIExpression()), !dbg !17
  %3 = bitcast %struct.AA* %1 to i8*, !dbg !17
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %3, i8* align 4 bitcast (%struct.AA* @__const.main.a to i8*), i64 8, i1 false), !dbg !17
  call void @llvm.dbg.declare(metadata %struct.AA.0* %2, metadata !18, metadata !DIExpression()), !dbg !24
  %4 = bitcast %struct.AA.0* %2 to i8*, !dbg !24
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %4, i8* align 8 bitcast (%struct.AA.0* @__const.main.a.1 to i8*), i64 8, i1 false), !dbg !24
  ret i32 0, !dbg !25
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
!1 = !DIFile(filename: "scope2.c", directory: "/home/rxia/git/compiler/ca/test/type")
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
!12 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "AA", file: !1, line: 1, size: 64, elements: !13)
!13 = !{!14, !15}
!14 = !DIDerivedType(tag: DW_TAG_member, name: "f1", scope: !12, file: !1, line: 2, baseType: !10, size: 32)
!15 = !DIDerivedType(tag: DW_TAG_member, name: "f2", scope: !12, file: !1, line: 3, baseType: !16, size: 8, offset: 32)
!16 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!17 = !DILocation(line: 7, column: 15, scope: !7)
!18 = !DILocalVariable(name: "a", scope: !19, file: !1, line: 12, type: !20)
!19 = distinct !DILexicalBlock(scope: !7, file: !1, line: 8, column: 5)
!20 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "AA", scope: !7, file: !1, line: 9, size: 64, elements: !21)
!21 = !{!22}
!22 = !DIDerivedType(tag: DW_TAG_member, name: "f1", scope: !20, file: !1, line: 10, baseType: !23, size: 64)
!23 = !DIBasicType(name: "double", size: 64, encoding: DW_ATE_float)
!24 = !DILocation(line: 12, column: 12, scope: !19)
!25 = !DILocation(line: 15, column: 1, scope: !7)
