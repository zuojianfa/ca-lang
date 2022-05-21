; ModuleID = 'short3.ca'
source_filename = "short3.ca"

%AA = type { i8, i16, i32, i64, i8, i16, i32, i64 }

@0 = private unnamed_addr constant [3 x i8] c"AA\00", align 1
@1 = private unnamed_addr constant [6 x i8] c"%s { \00", align 1
@2 = private unnamed_addr constant [3 x i8] c"f1\00", align 1
@3 = private unnamed_addr constant [5 x i8] c"%s: \00", align 1
@4 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@5 = private unnamed_addr constant [3 x i8] c", \00", align 1
@6 = private unnamed_addr constant [3 x i8] c"f2\00", align 1
@7 = private unnamed_addr constant [5 x i8] c"%s: \00", align 1
@8 = private unnamed_addr constant [3 x i8] c"%u\00", align 1
@9 = private unnamed_addr constant [3 x i8] c", \00", align 1
@10 = private unnamed_addr constant [3 x i8] c"f3\00", align 1
@11 = private unnamed_addr constant [5 x i8] c"%s: \00", align 1
@12 = private unnamed_addr constant [3 x i8] c"%u\00", align 1
@13 = private unnamed_addr constant [3 x i8] c", \00", align 1
@14 = private unnamed_addr constant [3 x i8] c"f4\00", align 1
@15 = private unnamed_addr constant [5 x i8] c"%s: \00", align 1
@16 = private unnamed_addr constant [4 x i8] c"%lu\00", align 1
@17 = private unnamed_addr constant [3 x i8] c", \00", align 1
@18 = private unnamed_addr constant [4 x i8] c"f11\00", align 1
@19 = private unnamed_addr constant [5 x i8] c"%s: \00", align 1
@20 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@21 = private unnamed_addr constant [3 x i8] c", \00", align 1
@22 = private unnamed_addr constant [4 x i8] c"f12\00", align 1
@23 = private unnamed_addr constant [5 x i8] c"%s: \00", align 1
@24 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@25 = private unnamed_addr constant [3 x i8] c", \00", align 1
@26 = private unnamed_addr constant [4 x i8] c"f13\00", align 1
@27 = private unnamed_addr constant [5 x i8] c"%s: \00", align 1
@28 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@29 = private unnamed_addr constant [3 x i8] c", \00", align 1
@30 = private unnamed_addr constant [4 x i8] c"f14\00", align 1
@31 = private unnamed_addr constant [5 x i8] c"%s: \00", align 1
@32 = private unnamed_addr constant [4 x i8] c"%ld\00", align 1
@33 = private unnamed_addr constant [3 x i8] c" }\00", align 1
@34 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@35 = private unnamed_addr constant [22 x i8] c"size = %lu, type: %s\0A\00", align 1
@36 = private unnamed_addr constant [65 x i8] c"t:{AA;f1:u8,f2:u16,f3:u32,f4:u64,f11:i8,f12:i16,f13:i32,f14:i64}\00", align 1

declare i32 @printf(i8*, ...)

define void @main() !dbg !4 {
entry:
  %a = alloca %AA, align 8
  %0 = alloca %AA, align 8
  %1 = getelementptr %AA, %AA* %0, i32 0, i32 0, !dbg !28
  store volatile i8 33, i8* %1, align 1, !dbg !28
  %2 = getelementptr %AA, %AA* %0, i32 0, i32 1, !dbg !28
  store volatile i16 -1, i16* %2, align 2, !dbg !28
  %3 = getelementptr %AA, %AA* %0, i32 0, i32 2, !dbg !28
  store volatile i32 65535, i32* %3, align 4, !dbg !28
  %4 = getelementptr %AA, %AA* %0, i32 0, i32 3, !dbg !28
  store volatile i64 33, i64* %4, align 4, !dbg !28
  %5 = getelementptr %AA, %AA* %0, i32 0, i32 4, !dbg !28
  store volatile i8 67, i8* %5, align 1, !dbg !28
  %6 = getelementptr %AA, %AA* %0, i32 0, i32 5, !dbg !28
  store volatile i16 4324, i16* %6, align 2, !dbg !28
  %7 = getelementptr %AA, %AA* %0, i32 0, i32 6, !dbg !28
  store volatile i32 432432, i32* %7, align 4, !dbg !28
  %8 = getelementptr %AA, %AA* %0, i32 0, i32 7, !dbg !28
  store volatile i64 32132132, i64* %8, align 4, !dbg !28
  %9 = bitcast %AA* %a to i8*, !dbg !28
  %10 = bitcast %AA* %0 to i8*, !dbg !28
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %9, i8* align 8 %10, i64 32, i1 false), !dbg !28
  call void @llvm.dbg.declare(metadata %AA* %a, metadata !9, metadata !DIExpression()), !dbg !29
  %load = load %AA, %AA* %a, align 4, !dbg !30
  %n = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @1, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0)), !dbg !30
  %n1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @3, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @2, i32 0, i32 0)), !dbg !30
  %11 = extractvalue %AA %load, 0, !dbg !30
  %n2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i8 %11), !dbg !30
  %n3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0)), !dbg !30
  %n4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @7, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @6, i32 0, i32 0)), !dbg !30
  %12 = extractvalue %AA %load, 1, !dbg !30
  %n5 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @8, i32 0, i32 0), i16 %12), !dbg !30
  %n6 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @9, i32 0, i32 0)), !dbg !30
  %n7 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @11, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @10, i32 0, i32 0)), !dbg !30
  %13 = extractvalue %AA %load, 2, !dbg !30
  %n8 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @12, i32 0, i32 0), i32 %13), !dbg !30
  %n9 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @13, i32 0, i32 0)), !dbg !30
  %n10 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @15, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @14, i32 0, i32 0)), !dbg !30
  %14 = extractvalue %AA %load, 3, !dbg !30
  %n11 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @16, i32 0, i32 0), i64 %14), !dbg !30
  %n12 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @17, i32 0, i32 0)), !dbg !30
  %n13 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @19, i32 0, i32 0), i8* getelementptr inbounds ([4 x i8], [4 x i8]* @18, i32 0, i32 0)), !dbg !30
  %15 = extractvalue %AA %load, 4, !dbg !30
  %n14 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @20, i32 0, i32 0), i8 %15), !dbg !30
  %n15 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @21, i32 0, i32 0)), !dbg !30
  %n16 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @23, i32 0, i32 0), i8* getelementptr inbounds ([4 x i8], [4 x i8]* @22, i32 0, i32 0)), !dbg !30
  %16 = extractvalue %AA %load, 5, !dbg !30
  %n17 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @24, i32 0, i32 0), i16 %16), !dbg !30
  %n18 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @25, i32 0, i32 0)), !dbg !30
  %n19 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @27, i32 0, i32 0), i8* getelementptr inbounds ([4 x i8], [4 x i8]* @26, i32 0, i32 0)), !dbg !30
  %17 = extractvalue %AA %load, 6, !dbg !30
  %n20 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @28, i32 0, i32 0), i32 %17), !dbg !30
  %n21 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @29, i32 0, i32 0)), !dbg !30
  %n22 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @31, i32 0, i32 0), i8* getelementptr inbounds ([4 x i8], [4 x i8]* @30, i32 0, i32 0)), !dbg !30
  %18 = extractvalue %AA %load, 7, !dbg !30
  %n23 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @32, i32 0, i32 0), i64 %18), !dbg !30
  %n24 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @33, i32 0, i32 0)), !dbg !30
  %n25 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @34, i32 0, i32 0), i8 10), !dbg !31
  %n26 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @35, i32 0, i32 0), i64 32, i8* getelementptr inbounds ([65 x i8], [65 x i8]* @36, i32 0, i32 0)), !dbg !32
  br label %ret, !dbg !33

ret:                                              ; preds = %entry
  ret void, !dbg !33
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
!2 = !DIFile(filename: "short3.ca", directory: ".")
!3 = !{}
!4 = distinct !DISubprogram(name: "main", scope: !2, file: !2, line: 12, type: !5, scopeLine: 12, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !1, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!8 = !{!9}
!9 = !DILocalVariable(name: "a", scope: !4, file: !2, line: 22, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "AA", scope: !4, file: !2, line: 5, size: 256, elements: !11)
!11 = !{!12, !14, !16, !18, !20, !22, !24, !26}
!12 = !DIDerivedType(tag: DW_TAG_member, name: "f1", scope: !10, file: !2, line: 5, baseType: !13, size: 8)
!13 = !DIBasicType(name: "u8", size: 8, encoding: DW_ATE_unsigned_char)
!14 = !DIDerivedType(tag: DW_TAG_member, name: "f2", scope: !10, file: !2, line: 5, baseType: !15, size: 16, offset: 16)
!15 = !DIBasicType(name: "u16", size: 16, encoding: DW_ATE_unsigned)
!16 = !DIDerivedType(tag: DW_TAG_member, name: "f3", scope: !10, file: !2, line: 5, baseType: !17, size: 32, offset: 32)
!17 = !DIBasicType(name: "u32", size: 32, encoding: DW_ATE_unsigned)
!18 = !DIDerivedType(tag: DW_TAG_member, name: "f4", scope: !10, file: !2, line: 5, baseType: !19, size: 64, offset: 64)
!19 = !DIBasicType(name: "u64", size: 64, encoding: DW_ATE_unsigned)
!20 = !DIDerivedType(tag: DW_TAG_member, name: "f11", scope: !10, file: !2, line: 5, baseType: !21, size: 8, offset: 128)
!21 = !DIBasicType(name: "i8", size: 8, encoding: DW_ATE_signed_char)
!22 = !DIDerivedType(tag: DW_TAG_member, name: "f12", scope: !10, file: !2, line: 5, baseType: !23, size: 16, offset: 144)
!23 = !DIBasicType(name: "i16", size: 16, encoding: DW_ATE_signed)
!24 = !DIDerivedType(tag: DW_TAG_member, name: "f13", scope: !10, file: !2, line: 5, baseType: !25, size: 32, offset: 160)
!25 = !DIBasicType(name: "i32", size: 32, encoding: DW_ATE_signed)
!26 = !DIDerivedType(tag: DW_TAG_member, name: "f14", scope: !10, file: !2, line: 5, baseType: !27, size: 64, offset: 192)
!27 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!28 = !DILocation(line: 21, column: 24, scope: !4)
!29 = !DILocation(line: 22, scope: !4)
!30 = !DILocation(line: 24, column: 12, scope: !4)
!31 = !DILocation(line: 24, column: 24, scope: !4)
!32 = !DILocation(line: 25, column: 27, scope: !4)
!33 = !DILocation(line: 26, column: 1, scope: !4)
