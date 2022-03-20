; ModuleID = 'struct_array3.ca'
source_filename = "struct_array3.ca"

%AA = type { i32, [3 x i32] }
%A1 = type { double, %AA }
%A2 = type { [2 x [2 x %AA]] }

@0 = private unnamed_addr constant [3 x i8] c"AA\00", align 1
@1 = private unnamed_addr constant [6 x i8] c"%s { \00", align 1
@2 = private unnamed_addr constant [6 x i8] c"aa_f1\00", align 1
@3 = private unnamed_addr constant [5 x i8] c"%s: \00", align 1
@4 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@5 = private unnamed_addr constant [3 x i8] c", \00", align 1
@6 = private unnamed_addr constant [6 x i8] c"aa_f2\00", align 1
@7 = private unnamed_addr constant [5 x i8] c"%s: \00", align 1
@8 = private unnamed_addr constant [2 x i8] c"[\00", align 1
@9 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@10 = private unnamed_addr constant [3 x i8] c", \00", align 1
@11 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@12 = private unnamed_addr constant [3 x i8] c", \00", align 1
@13 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@14 = private unnamed_addr constant [2 x i8] c"]\00", align 1
@15 = private unnamed_addr constant [3 x i8] c" }\00", align 1
@16 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@17 = private unnamed_addr constant [3 x i8] c"A1\00", align 1
@18 = private unnamed_addr constant [6 x i8] c"%s { \00", align 1
@19 = private unnamed_addr constant [6 x i8] c"a1_f1\00", align 1
@20 = private unnamed_addr constant [5 x i8] c"%s: \00", align 1
@21 = private unnamed_addr constant [4 x i8] c"%lf\00", align 1
@22 = private unnamed_addr constant [3 x i8] c", \00", align 1
@23 = private unnamed_addr constant [6 x i8] c"a1_f2\00", align 1
@24 = private unnamed_addr constant [5 x i8] c"%s: \00", align 1
@25 = private unnamed_addr constant [3 x i8] c"AA\00", align 1
@26 = private unnamed_addr constant [6 x i8] c"%s { \00", align 1
@27 = private unnamed_addr constant [6 x i8] c"aa_f1\00", align 1
@28 = private unnamed_addr constant [5 x i8] c"%s: \00", align 1
@29 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@30 = private unnamed_addr constant [3 x i8] c", \00", align 1
@31 = private unnamed_addr constant [6 x i8] c"aa_f2\00", align 1
@32 = private unnamed_addr constant [5 x i8] c"%s: \00", align 1
@33 = private unnamed_addr constant [2 x i8] c"[\00", align 1
@34 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@35 = private unnamed_addr constant [3 x i8] c", \00", align 1
@36 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@37 = private unnamed_addr constant [3 x i8] c", \00", align 1
@38 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@39 = private unnamed_addr constant [2 x i8] c"]\00", align 1
@40 = private unnamed_addr constant [3 x i8] c" }\00", align 1
@41 = private unnamed_addr constant [3 x i8] c" }\00", align 1
@42 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@43 = private unnamed_addr constant [3 x i8] c"A2\00", align 1
@44 = private unnamed_addr constant [6 x i8] c"%s { \00", align 1
@45 = private unnamed_addr constant [4 x i8] c"arr\00", align 1
@46 = private unnamed_addr constant [5 x i8] c"%s: \00", align 1
@47 = private unnamed_addr constant [2 x i8] c"[\00", align 1
@48 = private unnamed_addr constant [2 x i8] c"[\00", align 1
@49 = private unnamed_addr constant [3 x i8] c"AA\00", align 1
@50 = private unnamed_addr constant [6 x i8] c"%s { \00", align 1
@51 = private unnamed_addr constant [6 x i8] c"aa_f1\00", align 1
@52 = private unnamed_addr constant [5 x i8] c"%s: \00", align 1
@53 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@54 = private unnamed_addr constant [3 x i8] c", \00", align 1
@55 = private unnamed_addr constant [6 x i8] c"aa_f2\00", align 1
@56 = private unnamed_addr constant [5 x i8] c"%s: \00", align 1
@57 = private unnamed_addr constant [2 x i8] c"[\00", align 1
@58 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@59 = private unnamed_addr constant [3 x i8] c", \00", align 1
@60 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@61 = private unnamed_addr constant [3 x i8] c", \00", align 1
@62 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@63 = private unnamed_addr constant [2 x i8] c"]\00", align 1
@64 = private unnamed_addr constant [3 x i8] c" }\00", align 1
@65 = private unnamed_addr constant [3 x i8] c", \00", align 1
@66 = private unnamed_addr constant [3 x i8] c"AA\00", align 1
@67 = private unnamed_addr constant [6 x i8] c"%s { \00", align 1
@68 = private unnamed_addr constant [6 x i8] c"aa_f1\00", align 1
@69 = private unnamed_addr constant [5 x i8] c"%s: \00", align 1
@70 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@71 = private unnamed_addr constant [3 x i8] c", \00", align 1
@72 = private unnamed_addr constant [6 x i8] c"aa_f2\00", align 1
@73 = private unnamed_addr constant [5 x i8] c"%s: \00", align 1
@74 = private unnamed_addr constant [2 x i8] c"[\00", align 1
@75 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@76 = private unnamed_addr constant [3 x i8] c", \00", align 1
@77 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@78 = private unnamed_addr constant [3 x i8] c", \00", align 1
@79 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@80 = private unnamed_addr constant [2 x i8] c"]\00", align 1
@81 = private unnamed_addr constant [3 x i8] c" }\00", align 1
@82 = private unnamed_addr constant [2 x i8] c"]\00", align 1
@83 = private unnamed_addr constant [3 x i8] c", \00", align 1
@84 = private unnamed_addr constant [2 x i8] c"[\00", align 1
@85 = private unnamed_addr constant [3 x i8] c"AA\00", align 1
@86 = private unnamed_addr constant [6 x i8] c"%s { \00", align 1
@87 = private unnamed_addr constant [6 x i8] c"aa_f1\00", align 1
@88 = private unnamed_addr constant [5 x i8] c"%s: \00", align 1
@89 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@90 = private unnamed_addr constant [3 x i8] c", \00", align 1
@91 = private unnamed_addr constant [6 x i8] c"aa_f2\00", align 1
@92 = private unnamed_addr constant [5 x i8] c"%s: \00", align 1
@93 = private unnamed_addr constant [2 x i8] c"[\00", align 1
@94 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@95 = private unnamed_addr constant [3 x i8] c", \00", align 1
@96 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@97 = private unnamed_addr constant [3 x i8] c", \00", align 1
@98 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@99 = private unnamed_addr constant [2 x i8] c"]\00", align 1
@100 = private unnamed_addr constant [3 x i8] c" }\00", align 1
@101 = private unnamed_addr constant [3 x i8] c", \00", align 1
@102 = private unnamed_addr constant [3 x i8] c"AA\00", align 1
@103 = private unnamed_addr constant [6 x i8] c"%s { \00", align 1
@104 = private unnamed_addr constant [6 x i8] c"aa_f1\00", align 1
@105 = private unnamed_addr constant [5 x i8] c"%s: \00", align 1
@106 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@107 = private unnamed_addr constant [3 x i8] c", \00", align 1
@108 = private unnamed_addr constant [6 x i8] c"aa_f2\00", align 1
@109 = private unnamed_addr constant [5 x i8] c"%s: \00", align 1
@110 = private unnamed_addr constant [2 x i8] c"[\00", align 1
@111 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@112 = private unnamed_addr constant [3 x i8] c", \00", align 1
@113 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@114 = private unnamed_addr constant [3 x i8] c", \00", align 1
@115 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@116 = private unnamed_addr constant [2 x i8] c"]\00", align 1
@117 = private unnamed_addr constant [3 x i8] c" }\00", align 1
@118 = private unnamed_addr constant [2 x i8] c"]\00", align 1
@119 = private unnamed_addr constant [2 x i8] c"]\00", align 1
@120 = private unnamed_addr constant [3 x i8] c" }\00", align 1
@121 = private unnamed_addr constant [3 x i8] c"%c\00", align 1

declare i32 @printf(i8*, ...)

define void @main() !dbg !4 {
entry:
  %0 = alloca [3 x i32], align 4, !dbg !32
  %1 = getelementptr [3 x i32], [3 x i32]* %0, i32 0, i64 0, !dbg !32
  store volatile i32 1, i32* %1, align 4, !dbg !32
  %2 = getelementptr [3 x i32], [3 x i32]* %0, i32 0, i64 1, !dbg !32
  store volatile i32 2, i32* %2, align 4, !dbg !32
  %3 = getelementptr [3 x i32], [3 x i32]* %0, i32 0, i64 2, !dbg !32
  store volatile i32 3, i32* %3, align 4, !dbg !32
  %4 = alloca %AA, align 8, !dbg !32
  %5 = getelementptr %AA, %AA* %4, i32 0, i32 0, !dbg !32
  store volatile i32 321, i32* %5, align 4, !dbg !32
  %6 = getelementptr %AA, %AA* %4, i32 0, i32 1, !dbg !32
  %7 = bitcast [3 x i32]* %6 to i8*, !dbg !32
  %8 = bitcast [3 x i32]* %0 to i8*, !dbg !32
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %7, i8* align 1 %8, i64 12, i1 false), !dbg !32
  %aa = alloca %AA, align 8, !dbg !32
  %9 = bitcast %AA* %aa to i8*, !dbg !32
  %10 = bitcast %AA* %4 to i8*, !dbg !32
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %9, i8* align 8 %10, i64 16, i1 false), !dbg !32
  call void @llvm.dbg.declare(metadata %AA* %aa, metadata !9, metadata !DIExpression()), !dbg !33
  %11 = alloca %A1, align 8, !dbg !34
  %12 = getelementptr %A1, %A1* %11, i32 0, i32 0, !dbg !34
  store volatile double 3.141590e+00, double* %12, align 8, !dbg !34
  %13 = getelementptr %A1, %A1* %11, i32 0, i32 1, !dbg !34
  %14 = bitcast %AA* %13 to i8*, !dbg !34
  %15 = bitcast %AA* %aa to i8*, !dbg !34
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %14, i8* align 1 %15, i64 16, i1 false), !dbg !34
  %a1 = alloca %A1, align 8, !dbg !34
  %16 = bitcast %A1* %a1 to i8*, !dbg !34
  %17 = bitcast %A1* %11 to i8*, !dbg !34
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %16, i8* align 8 %17, i64 24, i1 false), !dbg !34
  call void @llvm.dbg.declare(metadata %A1* %a1, metadata !18, metadata !DIExpression()), !dbg !35
  %18 = alloca [2 x %AA], align 8, !dbg !36
  %19 = getelementptr [2 x %AA], [2 x %AA]* %18, i32 0, i64 0, !dbg !36
  %20 = bitcast %AA* %19 to i8*, !dbg !36
  %21 = bitcast %AA* %aa to i8*, !dbg !36
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %20, i8* align 1 %21, i64 16, i1 false), !dbg !36
  %22 = getelementptr [2 x %AA], [2 x %AA]* %18, i32 0, i64 1, !dbg !36
  %23 = bitcast %AA* %22 to i8*, !dbg !36
  %24 = bitcast %AA* %aa to i8*, !dbg !36
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %23, i8* align 1 %24, i64 16, i1 false), !dbg !36
  %25 = alloca [2 x %AA], align 8, !dbg !36
  %26 = getelementptr [2 x %AA], [2 x %AA]* %25, i32 0, i64 0, !dbg !36
  %27 = bitcast %AA* %26 to i8*, !dbg !36
  %28 = bitcast %AA* %aa to i8*, !dbg !36
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %27, i8* align 1 %28, i64 16, i1 false), !dbg !36
  %29 = getelementptr [2 x %AA], [2 x %AA]* %25, i32 0, i64 1, !dbg !36
  %30 = bitcast %AA* %29 to i8*, !dbg !36
  %31 = bitcast %AA* %aa to i8*, !dbg !36
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %30, i8* align 1 %31, i64 16, i1 false), !dbg !36
  %32 = alloca [2 x [2 x %AA]], align 8, !dbg !36
  %33 = getelementptr [2 x [2 x %AA]], [2 x [2 x %AA]]* %32, i32 0, i64 0, !dbg !36
  %34 = bitcast [2 x %AA]* %33 to i8*, !dbg !36
  %35 = bitcast [2 x %AA]* %18 to i8*, !dbg !36
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %34, i8* align 1 %35, i64 32, i1 false), !dbg !36
  %36 = getelementptr [2 x [2 x %AA]], [2 x [2 x %AA]]* %32, i32 0, i64 1, !dbg !36
  %37 = bitcast [2 x %AA]* %36 to i8*, !dbg !36
  %38 = bitcast [2 x %AA]* %25 to i8*, !dbg !36
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %37, i8* align 1 %38, i64 32, i1 false), !dbg !36
  %39 = alloca %A2, align 8, !dbg !36
  %40 = getelementptr %A2, %A2* %39, i32 0, i32 0, !dbg !36
  %41 = bitcast [2 x [2 x %AA]]* %40 to i8*, !dbg !36
  %42 = bitcast [2 x [2 x %AA]]* %32 to i8*, !dbg !36
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %41, i8* align 1 %42, i64 64, i1 false), !dbg !36
  %a2 = alloca %A2, align 8, !dbg !36
  %43 = bitcast %A2* %a2 to i8*, !dbg !36
  %44 = bitcast %A2* %39 to i8*, !dbg !36
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %43, i8* align 8 %44, i64 64, i1 false), !dbg !36
  call void @llvm.dbg.declare(metadata %A2* %a2, metadata !24, metadata !DIExpression()), !dbg !37
  %load = load %AA, %AA* %aa, align 4, !dbg !36
  %n = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @1, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0)), !dbg !36
  %n1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @3, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @2, i32 0, i32 0)), !dbg !36
  %45 = extractvalue %AA %load, 0, !dbg !36
  %n2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32 %45), !dbg !36
  %n3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0)), !dbg !36
  %n4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @7, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @6, i32 0, i32 0)), !dbg !36
  %46 = extractvalue %AA %load, 1, !dbg !36
  %n5 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @8, i32 0, i32 0)), !dbg !36
  %47 = extractvalue [3 x i32] %46, 0, !dbg !36
  %n6 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @9, i32 0, i32 0), i32 %47), !dbg !36
  %n7 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @10, i32 0, i32 0)), !dbg !36
  %48 = extractvalue [3 x i32] %46, 1, !dbg !36
  %n8 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @11, i32 0, i32 0), i32 %48), !dbg !36
  %n9 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @12, i32 0, i32 0)), !dbg !36
  %49 = extractvalue [3 x i32] %46, 2, !dbg !36
  %n10 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @13, i32 0, i32 0), i32 %49), !dbg !36
  %n11 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @14, i32 0, i32 0)), !dbg !36
  %n12 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @15, i32 0, i32 0)), !dbg !36
  %n13 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @16, i32 0, i32 0), i8 10), !dbg !38
  %load14 = load %A1, %A1* %a1, align 8, !dbg !38
  %n15 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @18, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @17, i32 0, i32 0)), !dbg !38
  %n16 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @20, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @19, i32 0, i32 0)), !dbg !38
  %50 = extractvalue %A1 %load14, 0, !dbg !38
  %n17 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @21, i32 0, i32 0), double %50), !dbg !38
  %n18 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @22, i32 0, i32 0)), !dbg !38
  %n19 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @24, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @23, i32 0, i32 0)), !dbg !38
  %51 = extractvalue %A1 %load14, 1, !dbg !38
  %n20 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @26, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @25, i32 0, i32 0)), !dbg !38
  %n21 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @28, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @27, i32 0, i32 0)), !dbg !38
  %52 = extractvalue %AA %51, 0, !dbg !38
  %n22 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @29, i32 0, i32 0), i32 %52), !dbg !38
  %n23 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @30, i32 0, i32 0)), !dbg !38
  %n24 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @32, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @31, i32 0, i32 0)), !dbg !38
  %53 = extractvalue %AA %51, 1, !dbg !38
  %n25 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @33, i32 0, i32 0)), !dbg !38
  %54 = extractvalue [3 x i32] %53, 0, !dbg !38
  %n26 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @34, i32 0, i32 0), i32 %54), !dbg !38
  %n27 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @35, i32 0, i32 0)), !dbg !38
  %55 = extractvalue [3 x i32] %53, 1, !dbg !38
  %n28 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @36, i32 0, i32 0), i32 %55), !dbg !38
  %n29 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @37, i32 0, i32 0)), !dbg !38
  %56 = extractvalue [3 x i32] %53, 2, !dbg !38
  %n30 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @38, i32 0, i32 0), i32 %56), !dbg !38
  %n31 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @39, i32 0, i32 0)), !dbg !38
  %n32 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @40, i32 0, i32 0)), !dbg !38
  %n33 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @41, i32 0, i32 0)), !dbg !38
  %n34 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @42, i32 0, i32 0), i8 10), !dbg !39
  %load35 = load %A2, %A2* %a2, align 4, !dbg !39
  %n36 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @44, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @43, i32 0, i32 0)), !dbg !39
  %n37 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @46, i32 0, i32 0), i8* getelementptr inbounds ([4 x i8], [4 x i8]* @45, i32 0, i32 0)), !dbg !39
  %57 = extractvalue %A2 %load35, 0, !dbg !39
  %n38 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @47, i32 0, i32 0)), !dbg !39
  %58 = extractvalue [2 x [2 x %AA]] %57, 0, !dbg !39
  %n39 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @48, i32 0, i32 0)), !dbg !39
  %59 = extractvalue [2 x %AA] %58, 0, !dbg !39
  %n40 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @50, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @49, i32 0, i32 0)), !dbg !39
  %n41 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @52, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @51, i32 0, i32 0)), !dbg !39
  %60 = extractvalue %AA %59, 0, !dbg !39
  %n42 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @53, i32 0, i32 0), i32 %60), !dbg !39
  %n43 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @54, i32 0, i32 0)), !dbg !39
  %n44 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @56, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @55, i32 0, i32 0)), !dbg !39
  %61 = extractvalue %AA %59, 1, !dbg !39
  %n45 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @57, i32 0, i32 0)), !dbg !39
  %62 = extractvalue [3 x i32] %61, 0, !dbg !39
  %n46 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @58, i32 0, i32 0), i32 %62), !dbg !39
  %n47 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @59, i32 0, i32 0)), !dbg !39
  %63 = extractvalue [3 x i32] %61, 1, !dbg !39
  %n48 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @60, i32 0, i32 0), i32 %63), !dbg !39
  %n49 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @61, i32 0, i32 0)), !dbg !39
  %64 = extractvalue [3 x i32] %61, 2, !dbg !39
  %n50 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @62, i32 0, i32 0), i32 %64), !dbg !39
  %n51 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @63, i32 0, i32 0)), !dbg !39
  %n52 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @64, i32 0, i32 0)), !dbg !39
  %n53 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @65, i32 0, i32 0)), !dbg !39
  %65 = extractvalue [2 x %AA] %58, 1, !dbg !39
  %n54 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @67, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @66, i32 0, i32 0)), !dbg !39
  %n55 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @69, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @68, i32 0, i32 0)), !dbg !39
  %66 = extractvalue %AA %65, 0, !dbg !39
  %n56 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @70, i32 0, i32 0), i32 %66), !dbg !39
  %n57 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @71, i32 0, i32 0)), !dbg !39
  %n58 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @73, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @72, i32 0, i32 0)), !dbg !39
  %67 = extractvalue %AA %65, 1, !dbg !39
  %n59 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @74, i32 0, i32 0)), !dbg !39
  %68 = extractvalue [3 x i32] %67, 0, !dbg !39
  %n60 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @75, i32 0, i32 0), i32 %68), !dbg !39
  %n61 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @76, i32 0, i32 0)), !dbg !39
  %69 = extractvalue [3 x i32] %67, 1, !dbg !39
  %n62 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @77, i32 0, i32 0), i32 %69), !dbg !39
  %n63 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @78, i32 0, i32 0)), !dbg !39
  %70 = extractvalue [3 x i32] %67, 2, !dbg !39
  %n64 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @79, i32 0, i32 0), i32 %70), !dbg !39
  %n65 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @80, i32 0, i32 0)), !dbg !39
  %n66 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @81, i32 0, i32 0)), !dbg !39
  %n67 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @82, i32 0, i32 0)), !dbg !39
  %n68 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @83, i32 0, i32 0)), !dbg !39
  %71 = extractvalue [2 x [2 x %AA]] %57, 1, !dbg !39
  %n69 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @84, i32 0, i32 0)), !dbg !39
  %72 = extractvalue [2 x %AA] %71, 0, !dbg !39
  %n70 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @86, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @85, i32 0, i32 0)), !dbg !39
  %n71 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @88, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @87, i32 0, i32 0)), !dbg !39
  %73 = extractvalue %AA %72, 0, !dbg !39
  %n72 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @89, i32 0, i32 0), i32 %73), !dbg !39
  %n73 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @90, i32 0, i32 0)), !dbg !39
  %n74 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @92, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @91, i32 0, i32 0)), !dbg !39
  %74 = extractvalue %AA %72, 1, !dbg !39
  %n75 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @93, i32 0, i32 0)), !dbg !39
  %75 = extractvalue [3 x i32] %74, 0, !dbg !39
  %n76 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @94, i32 0, i32 0), i32 %75), !dbg !39
  %n77 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @95, i32 0, i32 0)), !dbg !39
  %76 = extractvalue [3 x i32] %74, 1, !dbg !39
  %n78 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @96, i32 0, i32 0), i32 %76), !dbg !39
  %n79 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @97, i32 0, i32 0)), !dbg !39
  %77 = extractvalue [3 x i32] %74, 2, !dbg !39
  %n80 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @98, i32 0, i32 0), i32 %77), !dbg !39
  %n81 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @99, i32 0, i32 0)), !dbg !39
  %n82 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @100, i32 0, i32 0)), !dbg !39
  %n83 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @101, i32 0, i32 0)), !dbg !39
  %78 = extractvalue [2 x %AA] %71, 1, !dbg !39
  %n84 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @103, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @102, i32 0, i32 0)), !dbg !39
  %n85 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @105, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @104, i32 0, i32 0)), !dbg !39
  %79 = extractvalue %AA %78, 0, !dbg !39
  %n86 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @106, i32 0, i32 0), i32 %79), !dbg !39
  %n87 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @107, i32 0, i32 0)), !dbg !39
  %n88 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @109, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @108, i32 0, i32 0)), !dbg !39
  %80 = extractvalue %AA %78, 1, !dbg !39
  %n89 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @110, i32 0, i32 0)), !dbg !39
  %81 = extractvalue [3 x i32] %80, 0, !dbg !39
  %n90 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @111, i32 0, i32 0), i32 %81), !dbg !39
  %n91 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @112, i32 0, i32 0)), !dbg !39
  %82 = extractvalue [3 x i32] %80, 1, !dbg !39
  %n92 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @113, i32 0, i32 0), i32 %82), !dbg !39
  %n93 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @114, i32 0, i32 0)), !dbg !39
  %83 = extractvalue [3 x i32] %80, 2, !dbg !39
  %n94 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @115, i32 0, i32 0), i32 %83), !dbg !39
  %n95 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @116, i32 0, i32 0)), !dbg !39
  %n96 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @117, i32 0, i32 0)), !dbg !39
  %n97 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @118, i32 0, i32 0)), !dbg !39
  %n98 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @119, i32 0, i32 0)), !dbg !39
  %n99 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @120, i32 0, i32 0)), !dbg !39
  %n100 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @121, i32 0, i32 0), i8 10), !dbg !40
  br label %ret, !dbg !41

ret:                                              ; preds = %entry
  ret void, !dbg !41
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
!2 = !DIFile(filename: "struct_array3.ca", directory: ".")
!3 = !{}
!4 = distinct !DISubprogram(name: "main", scope: !2, file: !2, line: 15, type: !5, scopeLine: 15, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !1, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!8 = !{!9, !18, !24}
!9 = !DILocalVariable(name: "aa", scope: !4, file: !2, line: 16, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "AA", scope: !4, file: !2, line: 5, size: 128, elements: !11)
!11 = !{!12, !14}
!12 = !DIDerivedType(tag: DW_TAG_member, name: "aa_f1", scope: !10, file: !2, line: 5, baseType: !13, size: 32)
!13 = !DIBasicType(name: "i32", size: 32, encoding: DW_ATE_signed)
!14 = !DIDerivedType(tag: DW_TAG_member, name: "aa_f2", scope: !10, file: !2, line: 5, baseType: !15, size: 96, offset: 32)
!15 = !DICompositeType(tag: DW_TAG_array_type, baseType: !13, size: 96, elements: !16)
!16 = !{!17}
!17 = !DISubrange(count: 3, lowerBound: 0)
!18 = !DILocalVariable(name: "a1", scope: !4, file: !2, line: 17, type: !19)
!19 = !DICompositeType(tag: DW_TAG_structure_type, name: "A1", scope: !4, file: !2, line: 5, size: 192, elements: !20)
!20 = !{!21, !23}
!21 = !DIDerivedType(tag: DW_TAG_member, name: "a1_f1", scope: !19, file: !2, line: 5, baseType: !22, size: 64)
!22 = !DIBasicType(name: "f64", size: 64, encoding: DW_ATE_float)
!23 = !DIDerivedType(tag: DW_TAG_member, name: "a1_f2", scope: !19, file: !2, line: 5, baseType: !10, size: 128, offset: 64)
!24 = !DILocalVariable(name: "a2", scope: !4, file: !2, line: 18, type: !25)
!25 = !DICompositeType(tag: DW_TAG_structure_type, name: "A2", scope: !4, file: !2, line: 5, size: 512, elements: !26)
!26 = !{!27}
!27 = !DIDerivedType(tag: DW_TAG_member, name: "arr", scope: !25, file: !2, line: 5, baseType: !28, size: 512)
!28 = !DICompositeType(tag: DW_TAG_array_type, baseType: !29, size: 512, elements: !30)
!29 = !DICompositeType(tag: DW_TAG_array_type, baseType: !10, size: 256, elements: !30)
!30 = !{!31}
!31 = !DISubrange(count: 2, lowerBound: 0)
!32 = !DILocation(line: 16, column: 30, scope: !4)
!33 = !DILocation(line: 16, scope: !4)
!34 = !DILocation(line: 17, column: 24, scope: !4)
!35 = !DILocation(line: 17, scope: !4)
!36 = !DILocation(line: 18, column: 37, scope: !4)
!37 = !DILocation(line: 18, scope: !4)
!38 = !DILocation(line: 19, column: 25, scope: !4)
!39 = !DILocation(line: 20, column: 25, scope: !4)
!40 = !DILocation(line: 21, column: 25, scope: !4)
!41 = !DILocation(line: 22, column: 1, scope: !4)
