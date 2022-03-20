; ModuleID = 'struct_one1.c'
source_filename = "struct_one1.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.AA = type { i32, double, i8 }
%struct.A1 = type { double, i32, i8 }
%struct.A3 = type { %struct.AA, %struct.A1, %struct.C1 }
%struct.C1 = type { i8 }
%struct.A2 = type { %struct.AA, %struct.A1 }
%struct.A21 = type { i8, %struct.AA, %struct.A1 }
%struct.A22 = type { %struct.AA, %struct.A1, i8 }

@__const.main.a = private unnamed_addr constant %struct.AA { i32 32, double 4.430000e+01, i8 67 }, align 8
@__const.main.b = private unnamed_addr constant %struct.A1 { double 4.430000e+01, i32 32, i8 68 }, align 8
@__const.main.a3 = private unnamed_addr constant %struct.A3 { %struct.AA { i32 32, double 4.430000e+01, i8 67 }, %struct.A1 { double 4.430000e+01, i32 32, i8 68 }, %struct.C1 { i8 66 } }, align 8
@__const.main.c1 = private unnamed_addr constant %struct.C1 { i8 65 }, align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 !dbg !7 {
  %1 = alloca %struct.AA, align 8
  %2 = alloca %struct.A1, align 8
  %3 = alloca %struct.A2, align 8
  %4 = alloca %struct.A3, align 8
  %5 = alloca %struct.C1, align 1
  %6 = alloca %struct.A21, align 8
  %7 = alloca %struct.A22, align 8
  %8 = alloca double, align 8
  %9 = alloca i32, align 4
  %10 = alloca double, align 8
  %11 = alloca i8, align 1
  %12 = alloca double, align 8
  %13 = alloca i32, align 4
  %14 = alloca i32, align 4
  call void @llvm.dbg.declare(metadata %struct.AA* %1, metadata !11, metadata !DIExpression()), !dbg !19
  %15 = bitcast %struct.AA* %1 to i8*, !dbg !19
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %15, i8* align 8 bitcast (%struct.AA* @__const.main.a to i8*), i64 24, i1 false), !dbg !19
  call void @llvm.dbg.declare(metadata %struct.A1* %2, metadata !20, metadata !DIExpression()), !dbg !26
  %16 = bitcast %struct.A1* %2 to i8*, !dbg !26
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %16, i8* align 8 bitcast (%struct.A1* @__const.main.b to i8*), i64 16, i1 false), !dbg !26
  call void @llvm.dbg.declare(metadata %struct.A2* %3, metadata !27, metadata !DIExpression()), !dbg !32
  %17 = bitcast %struct.A2* %3 to i8*, !dbg !32
  call void @llvm.memset.p0i8.i64(i8* align 8 %17, i8 0, i64 40, i1 false), !dbg !32
  %18 = bitcast i8* %17 to %struct.A2*, !dbg !32
  %19 = getelementptr inbounds %struct.A2, %struct.A2* %18, i32 0, i32 0, !dbg !32
  %20 = getelementptr inbounds %struct.AA, %struct.AA* %19, i32 0, i32 0, !dbg !32
  store i32 32, i32* %20, align 8, !dbg !32
  %21 = getelementptr inbounds %struct.AA, %struct.AA* %19, i32 0, i32 1, !dbg !32
  store double 4.430000e+01, double* %21, align 8, !dbg !32
  %22 = getelementptr inbounds %struct.AA, %struct.AA* %19, i32 0, i32 2, !dbg !32
  store i8 67, i8* %22, align 8, !dbg !32
  %23 = getelementptr inbounds %struct.A2, %struct.A2* %18, i32 0, i32 1, !dbg !32
  %24 = getelementptr inbounds %struct.A1, %struct.A1* %23, i32 0, i32 0, !dbg !32
  store double 4.430000e+01, double* %24, align 8, !dbg !32
  %25 = getelementptr inbounds %struct.A1, %struct.A1* %23, i32 0, i32 1, !dbg !32
  store i32 32, i32* %25, align 8, !dbg !32
  %26 = getelementptr inbounds %struct.A1, %struct.A1* %23, i32 0, i32 2, !dbg !32
  store i8 68, i8* %26, align 4, !dbg !32
  call void @llvm.dbg.declare(metadata %struct.A3* %4, metadata !33, metadata !DIExpression()), !dbg !42
  %27 = bitcast %struct.A3* %4 to i8*, !dbg !42
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %27, i8* align 8 bitcast (%struct.A3* @__const.main.a3 to i8*), i64 48, i1 false), !dbg !42
  call void @llvm.dbg.declare(metadata %struct.C1* %5, metadata !43, metadata !DIExpression()), !dbg !44
  %28 = bitcast %struct.C1* %5 to i8*, !dbg !44
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %28, i8* align 1 getelementptr inbounds (%struct.C1, %struct.C1* @__const.main.c1, i32 0, i32 0), i64 1, i1 false), !dbg !44
  call void @llvm.dbg.declare(metadata %struct.A21* %6, metadata !45, metadata !DIExpression()), !dbg !51
  call void @llvm.dbg.declare(metadata %struct.A22* %7, metadata !52, metadata !DIExpression()), !dbg !58
  call void @llvm.dbg.declare(metadata double* %8, metadata !59, metadata !DIExpression()), !dbg !60
  call void @llvm.dbg.declare(metadata i32* %9, metadata !61, metadata !DIExpression()), !dbg !62
  call void @llvm.dbg.declare(metadata double* %10, metadata !63, metadata !DIExpression()), !dbg !64
  call void @llvm.dbg.declare(metadata i8* %11, metadata !65, metadata !DIExpression()), !dbg !66
  call void @llvm.dbg.declare(metadata double* %12, metadata !67, metadata !DIExpression()), !dbg !68
  call void @llvm.dbg.declare(metadata i32* %13, metadata !69, metadata !DIExpression()), !dbg !70
  store i32 24, i32* %13, align 4, !dbg !70
  call void @llvm.dbg.declare(metadata i32* %14, metadata !71, metadata !DIExpression()), !dbg !72
  store i32 16, i32* %14, align 4, !dbg !72
  ret i32 0, !dbg !73
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #2

; Function Attrs: argmemonly nofree nosync nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #3

attributes #0 = { noinline nounwind optnone uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { argmemonly nofree nosync nounwind willreturn }
attributes #3 = { argmemonly nofree nosync nounwind willreturn writeonly }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 12.0.0", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "struct_one1.c", directory: "/home/xrsh/git/compiler/ca/test/struct")
!2 = !{}
!3 = !{i32 7, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"clang version 12.0.0"}
!7 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 40, type: !8, scopeLine: 40, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DILocalVariable(name: "a", scope: !7, file: !1, line: 41, type: !12)
!12 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "AA", file: !1, line: 5, size: 192, elements: !13)
!13 = !{!14, !15, !17}
!14 = !DIDerivedType(tag: DW_TAG_member, name: "f1", scope: !12, file: !1, line: 6, baseType: !10, size: 32)
!15 = !DIDerivedType(tag: DW_TAG_member, name: "f2", scope: !12, file: !1, line: 7, baseType: !16, size: 64, offset: 64)
!16 = !DIBasicType(name: "double", size: 64, encoding: DW_ATE_float)
!17 = !DIDerivedType(tag: DW_TAG_member, name: "f3", scope: !12, file: !1, line: 8, baseType: !18, size: 8, offset: 128)
!18 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!19 = !DILocation(line: 41, column: 13, scope: !7)
!20 = !DILocalVariable(name: "b", scope: !7, file: !1, line: 42, type: !21)
!21 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "A1", file: !1, line: 11, size: 128, elements: !22)
!22 = !{!23, !24, !25}
!23 = !DIDerivedType(tag: DW_TAG_member, name: "f2", scope: !21, file: !1, line: 12, baseType: !16, size: 64)
!24 = !DIDerivedType(tag: DW_TAG_member, name: "f1", scope: !21, file: !1, line: 13, baseType: !10, size: 32, offset: 64)
!25 = !DIDerivedType(tag: DW_TAG_member, name: "f3", scope: !21, file: !1, line: 14, baseType: !18, size: 8, offset: 96)
!26 = !DILocation(line: 42, column: 13, scope: !7)
!27 = !DILocalVariable(name: "c", scope: !7, file: !1, line: 43, type: !28)
!28 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "A2", file: !1, line: 17, size: 320, elements: !29)
!29 = !{!30, !31}
!30 = !DIDerivedType(tag: DW_TAG_member, name: "aa", scope: !28, file: !1, line: 18, baseType: !12, size: 192)
!31 = !DIDerivedType(tag: DW_TAG_member, name: "a1", scope: !28, file: !1, line: 19, baseType: !21, size: 128, offset: 192)
!32 = !DILocation(line: 43, column: 13, scope: !7)
!33 = !DILocalVariable(name: "a3", scope: !7, file: !1, line: 44, type: !34)
!34 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "A3", file: !1, line: 34, size: 384, elements: !35)
!35 = !{!36, !37, !38}
!36 = !DIDerivedType(tag: DW_TAG_member, name: "aa", scope: !34, file: !1, line: 35, baseType: !12, size: 192)
!37 = !DIDerivedType(tag: DW_TAG_member, name: "a1", scope: !34, file: !1, line: 36, baseType: !21, size: 128, offset: 192)
!38 = !DIDerivedType(tag: DW_TAG_member, name: "c1", scope: !34, file: !1, line: 37, baseType: !39, size: 8, offset: 320)
!39 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "C1", file: !1, line: 1, size: 8, elements: !40)
!40 = !{!41}
!41 = !DIDerivedType(tag: DW_TAG_member, name: "f1", scope: !39, file: !1, line: 2, baseType: !18, size: 8)
!42 = !DILocation(line: 44, column: 13, scope: !7)
!43 = !DILocalVariable(name: "c1", scope: !7, file: !1, line: 45, type: !39)
!44 = !DILocation(line: 45, column: 13, scope: !7)
!45 = !DILocalVariable(name: "a21", scope: !7, file: !1, line: 47, type: !46)
!46 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "A21", file: !1, line: 22, size: 384, elements: !47)
!47 = !{!48, !49, !50}
!48 = !DIDerivedType(tag: DW_TAG_member, name: "c1", scope: !46, file: !1, line: 23, baseType: !18, size: 8)
!49 = !DIDerivedType(tag: DW_TAG_member, name: "aa", scope: !46, file: !1, line: 24, baseType: !12, size: 192, offset: 64)
!50 = !DIDerivedType(tag: DW_TAG_member, name: "a1", scope: !46, file: !1, line: 25, baseType: !21, size: 128, offset: 256)
!51 = !DILocation(line: 47, column: 14, scope: !7)
!52 = !DILocalVariable(name: "a22", scope: !7, file: !1, line: 48, type: !53)
!53 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "A22", file: !1, line: 28, size: 384, elements: !54)
!54 = !{!55, !56, !57}
!55 = !DIDerivedType(tag: DW_TAG_member, name: "aa", scope: !53, file: !1, line: 29, baseType: !12, size: 192)
!56 = !DIDerivedType(tag: DW_TAG_member, name: "a1", scope: !53, file: !1, line: 30, baseType: !21, size: 128, offset: 192)
!57 = !DIDerivedType(tag: DW_TAG_member, name: "c1", scope: !53, file: !1, line: 31, baseType: !18, size: 8, offset: 320)
!58 = !DILocation(line: 48, column: 14, scope: !7)
!59 = !DILocalVariable(name: "f1", scope: !7, file: !1, line: 50, type: !16)
!60 = !DILocation(line: 50, column: 10, scope: !7)
!61 = !DILocalVariable(name: "f2", scope: !7, file: !1, line: 51, type: !10)
!62 = !DILocation(line: 51, column: 7, scope: !7)
!63 = !DILocalVariable(name: "f21", scope: !7, file: !1, line: 52, type: !16)
!64 = !DILocation(line: 52, column: 10, scope: !7)
!65 = !DILocalVariable(name: "f3", scope: !7, file: !1, line: 53, type: !18)
!66 = !DILocation(line: 53, column: 8, scope: !7)
!67 = !DILocalVariable(name: "f4", scope: !7, file: !1, line: 54, type: !16)
!68 = !DILocation(line: 54, column: 10, scope: !7)
!69 = !DILocalVariable(name: "sa", scope: !7, file: !1, line: 56, type: !10)
!70 = !DILocation(line: 56, column: 7, scope: !7)
!71 = !DILocalVariable(name: "sb", scope: !7, file: !1, line: 57, type: !10)
!72 = !DILocation(line: 57, column: 7, scope: !7)
!73 = !DILocation(line: 58, column: 1, scope: !7)
