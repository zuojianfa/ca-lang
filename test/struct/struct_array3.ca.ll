; ModuleID = 'struct_array3.ca'
source_filename = "struct_array3.ca"

%A2 = type { [2 x [2 x %AA]] }
%AA = type { i32, [3 x i32] }
%A1 = type { double, %AA }

@0 = private unnamed_addr constant [3 x i8] c"AA\00", align 1
@1 = private unnamed_addr constant [6 x i8] c"%s { \00", align 1
@2 = private unnamed_addr constant [6 x i8] c"aa_f1\00", align 1
@3 = private unnamed_addr constant [5 x i8] c"%s: \00", align 1
@4 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@5 = private unnamed_addr constant [3 x i8] c", \00", align 1
@6 = private unnamed_addr constant [6 x i8] c"aa_f2\00", align 1
@7 = private unnamed_addr constant [2 x i8] c"[\00", align 1
@8 = private unnamed_addr constant [2 x i8] c"]\00", align 1
@9 = private unnamed_addr constant [3 x i8] c" }\00", align 1
@10 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@11 = private unnamed_addr constant [3 x i8] c"A1\00", align 1
@12 = private unnamed_addr constant [6 x i8] c"a1_f1\00", align 1
@13 = private unnamed_addr constant [4 x i8] c"%lf\00", align 1
@14 = private unnamed_addr constant [6 x i8] c"a1_f2\00", align 1
@15 = private unnamed_addr constant [3 x i8] c"A2\00", align 1
@16 = private unnamed_addr constant [4 x i8] c"arr\00", align 1

declare i32 @printf(i8*, ...)

define void @main() !dbg !4 {
entry:
  %a2 = alloca %A2, align 8
  %0 = alloca %A2, align 8
  %1 = alloca [2 x [2 x %AA]], align 8
  %2 = alloca [2 x %AA], align 8
  %3 = alloca [2 x %AA], align 8
  %a1 = alloca %A1, align 8
  %4 = alloca %A1, align 8
  %aa = alloca %AA, align 8
  %5 = alloca %AA, align 8
  %6 = alloca [3 x i32], align 4
  %7 = getelementptr [3 x i32], [3 x i32]* %6, i32 0, i64 0, !dbg !32
  store volatile i32 1, i32* %7, align 4, !dbg !32
  %8 = getelementptr [3 x i32], [3 x i32]* %6, i32 0, i64 1, !dbg !32
  store volatile i32 2, i32* %8, align 4, !dbg !32
  %9 = getelementptr [3 x i32], [3 x i32]* %6, i32 0, i64 2, !dbg !32
  store volatile i32 3, i32* %9, align 4, !dbg !32
  %10 = getelementptr %AA, %AA* %5, i32 0, i32 0, !dbg !32
  store volatile i32 321, i32* %10, align 4, !dbg !32
  %11 = getelementptr %AA, %AA* %5, i32 0, i32 1, !dbg !32
  %12 = bitcast [3 x i32]* %11 to i8*, !dbg !32
  %13 = bitcast [3 x i32]* %6 to i8*, !dbg !32
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %12, i8* align 1 %13, i64 12, i1 false), !dbg !32
  %14 = bitcast %AA* %aa to i8*, !dbg !32
  %15 = bitcast %AA* %5 to i8*, !dbg !32
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %14, i8* align 8 %15, i64 16, i1 false), !dbg !32
  call void @llvm.dbg.declare(metadata %AA* %aa, metadata !9, metadata !DIExpression()), !dbg !33
  %16 = getelementptr %A1, %A1* %4, i32 0, i32 0, !dbg !34
  store volatile double 3.141590e+00, double* %16, align 8, !dbg !34
  %17 = getelementptr %A1, %A1* %4, i32 0, i32 1, !dbg !34
  %18 = bitcast %AA* %17 to i8*, !dbg !34
  %19 = bitcast %AA* %aa to i8*, !dbg !34
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %18, i8* align 1 %19, i64 16, i1 false), !dbg !34
  %20 = bitcast %A1* %a1 to i8*, !dbg !34
  %21 = bitcast %A1* %4 to i8*, !dbg !34
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %20, i8* align 8 %21, i64 24, i1 false), !dbg !34
  call void @llvm.dbg.declare(metadata %A1* %a1, metadata !18, metadata !DIExpression()), !dbg !35
  %22 = getelementptr [2 x %AA], [2 x %AA]* %3, i32 0, i64 0, !dbg !36
  %23 = bitcast %AA* %22 to i8*, !dbg !36
  %24 = bitcast %AA* %aa to i8*, !dbg !36
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %23, i8* align 1 %24, i64 16, i1 false), !dbg !36
  %25 = getelementptr [2 x %AA], [2 x %AA]* %3, i32 0, i64 1, !dbg !36
  %26 = bitcast %AA* %25 to i8*, !dbg !36
  %27 = bitcast %AA* %aa to i8*, !dbg !36
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %26, i8* align 1 %27, i64 16, i1 false), !dbg !36
  %28 = getelementptr [2 x %AA], [2 x %AA]* %2, i32 0, i64 0, !dbg !36
  %29 = bitcast %AA* %28 to i8*, !dbg !36
  %30 = bitcast %AA* %aa to i8*, !dbg !36
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %29, i8* align 1 %30, i64 16, i1 false), !dbg !36
  %31 = getelementptr [2 x %AA], [2 x %AA]* %2, i32 0, i64 1, !dbg !36
  %32 = bitcast %AA* %31 to i8*, !dbg !36
  %33 = bitcast %AA* %aa to i8*, !dbg !36
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %32, i8* align 1 %33, i64 16, i1 false), !dbg !36
  %34 = getelementptr [2 x [2 x %AA]], [2 x [2 x %AA]]* %1, i32 0, i64 0, !dbg !36
  %35 = bitcast [2 x %AA]* %34 to i8*, !dbg !36
  %36 = bitcast [2 x %AA]* %3 to i8*, !dbg !36
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %35, i8* align 1 %36, i64 32, i1 false), !dbg !36
  %37 = getelementptr [2 x [2 x %AA]], [2 x [2 x %AA]]* %1, i32 0, i64 1, !dbg !36
  %38 = bitcast [2 x %AA]* %37 to i8*, !dbg !36
  %39 = bitcast [2 x %AA]* %2 to i8*, !dbg !36
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %38, i8* align 1 %39, i64 32, i1 false), !dbg !36
  %40 = getelementptr %A2, %A2* %0, i32 0, i32 0, !dbg !36
  %41 = bitcast [2 x [2 x %AA]]* %40 to i8*, !dbg !36
  %42 = bitcast [2 x [2 x %AA]]* %1 to i8*, !dbg !36
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %41, i8* align 1 %42, i64 64, i1 false), !dbg !36
  %43 = bitcast %A2* %a2 to i8*, !dbg !36
  %44 = bitcast %A2* %0 to i8*, !dbg !36
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %43, i8* align 8 %44, i64 64, i1 false), !dbg !36
  call void @llvm.dbg.declare(metadata %A2* %a2, metadata !24, metadata !DIExpression()), !dbg !37
  %load = load %AA, %AA* %aa, align 4, !dbg !38
  %n = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @1, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0)), !dbg !38
  %n1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @3, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @2, i32 0, i32 0)), !dbg !38
  %45 = extractvalue %AA %load, 0, !dbg !38
  %n2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32 %45), !dbg !38
  %n3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0)), !dbg !38
  %n4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @3, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @6, i32 0, i32 0)), !dbg !38
  %46 = extractvalue %AA %load, 1, !dbg !38
  %n5 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @7, i32 0, i32 0)), !dbg !38
  %47 = extractvalue [3 x i32] %46, 0, !dbg !38
  %n6 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32 %47), !dbg !38
  %n7 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0)), !dbg !38
  %48 = extractvalue [3 x i32] %46, 1, !dbg !38
  %n8 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32 %48), !dbg !38
  %n9 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0)), !dbg !38
  %49 = extractvalue [3 x i32] %46, 2, !dbg !38
  %n10 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32 %49), !dbg !38
  %n11 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @8, i32 0, i32 0)), !dbg !38
  %n12 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @9, i32 0, i32 0)), !dbg !38
  %n13 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @10, i32 0, i32 0), i8 10), !dbg !39
  %load14 = load %A1, %A1* %a1, align 8, !dbg !40
  %n15 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @1, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @11, i32 0, i32 0)), !dbg !40
  %n16 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @3, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @12, i32 0, i32 0)), !dbg !40
  %50 = extractvalue %A1 %load14, 0, !dbg !40
  %n17 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @13, i32 0, i32 0), double %50), !dbg !40
  %n18 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0)), !dbg !40
  %n19 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @3, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @14, i32 0, i32 0)), !dbg !40
  %51 = extractvalue %A1 %load14, 1, !dbg !40
  %n20 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @1, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0)), !dbg !40
  %n21 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @3, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @2, i32 0, i32 0)), !dbg !40
  %52 = extractvalue %AA %51, 0, !dbg !40
  %n22 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32 %52), !dbg !40
  %n23 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0)), !dbg !40
  %n24 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @3, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @6, i32 0, i32 0)), !dbg !40
  %53 = extractvalue %AA %51, 1, !dbg !40
  %n25 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @7, i32 0, i32 0)), !dbg !40
  %54 = extractvalue [3 x i32] %53, 0, !dbg !40
  %n26 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32 %54), !dbg !40
  %n27 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0)), !dbg !40
  %55 = extractvalue [3 x i32] %53, 1, !dbg !40
  %n28 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32 %55), !dbg !40
  %n29 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0)), !dbg !40
  %56 = extractvalue [3 x i32] %53, 2, !dbg !40
  %n30 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32 %56), !dbg !40
  %n31 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @8, i32 0, i32 0)), !dbg !40
  %n32 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @9, i32 0, i32 0)), !dbg !40
  %n33 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @9, i32 0, i32 0)), !dbg !40
  %n34 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @10, i32 0, i32 0), i8 10), !dbg !41
  %load35 = load %A2, %A2* %a2, align 4, !dbg !42
  %n36 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @1, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @15, i32 0, i32 0)), !dbg !42
  %n37 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @3, i32 0, i32 0), i8* getelementptr inbounds ([4 x i8], [4 x i8]* @16, i32 0, i32 0)), !dbg !42
  %57 = extractvalue %A2 %load35, 0, !dbg !42
  %n38 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @7, i32 0, i32 0)), !dbg !42
  %58 = extractvalue [2 x [2 x %AA]] %57, 0, !dbg !42
  %n39 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @7, i32 0, i32 0)), !dbg !42
  %59 = extractvalue [2 x %AA] %58, 0, !dbg !42
  %n40 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @1, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0)), !dbg !42
  %n41 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @3, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @2, i32 0, i32 0)), !dbg !42
  %60 = extractvalue %AA %59, 0, !dbg !42
  %n42 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32 %60), !dbg !42
  %n43 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0)), !dbg !42
  %n44 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @3, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @6, i32 0, i32 0)), !dbg !42
  %61 = extractvalue %AA %59, 1, !dbg !42
  %n45 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @7, i32 0, i32 0)), !dbg !42
  %62 = extractvalue [3 x i32] %61, 0, !dbg !42
  %n46 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32 %62), !dbg !42
  %n47 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0)), !dbg !42
  %63 = extractvalue [3 x i32] %61, 1, !dbg !42
  %n48 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32 %63), !dbg !42
  %n49 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0)), !dbg !42
  %64 = extractvalue [3 x i32] %61, 2, !dbg !42
  %n50 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32 %64), !dbg !42
  %n51 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @8, i32 0, i32 0)), !dbg !42
  %n52 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @9, i32 0, i32 0)), !dbg !42
  %n53 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0)), !dbg !42
  %65 = extractvalue [2 x %AA] %58, 1, !dbg !42
  %n54 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @1, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0)), !dbg !42
  %n55 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @3, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @2, i32 0, i32 0)), !dbg !42
  %66 = extractvalue %AA %65, 0, !dbg !42
  %n56 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32 %66), !dbg !42
  %n57 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0)), !dbg !42
  %n58 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @3, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @6, i32 0, i32 0)), !dbg !42
  %67 = extractvalue %AA %65, 1, !dbg !42
  %n59 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @7, i32 0, i32 0)), !dbg !42
  %68 = extractvalue [3 x i32] %67, 0, !dbg !42
  %n60 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32 %68), !dbg !42
  %n61 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0)), !dbg !42
  %69 = extractvalue [3 x i32] %67, 1, !dbg !42
  %n62 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32 %69), !dbg !42
  %n63 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0)), !dbg !42
  %70 = extractvalue [3 x i32] %67, 2, !dbg !42
  %n64 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32 %70), !dbg !42
  %n65 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @8, i32 0, i32 0)), !dbg !42
  %n66 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @9, i32 0, i32 0)), !dbg !42
  %n67 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @8, i32 0, i32 0)), !dbg !42
  %n68 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0)), !dbg !42
  %71 = extractvalue [2 x [2 x %AA]] %57, 1, !dbg !42
  %n69 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @7, i32 0, i32 0)), !dbg !42
  %72 = extractvalue [2 x %AA] %71, 0, !dbg !42
  %n70 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @1, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0)), !dbg !42
  %n71 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @3, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @2, i32 0, i32 0)), !dbg !42
  %73 = extractvalue %AA %72, 0, !dbg !42
  %n72 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32 %73), !dbg !42
  %n73 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0)), !dbg !42
  %n74 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @3, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @6, i32 0, i32 0)), !dbg !42
  %74 = extractvalue %AA %72, 1, !dbg !42
  %n75 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @7, i32 0, i32 0)), !dbg !42
  %75 = extractvalue [3 x i32] %74, 0, !dbg !42
  %n76 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32 %75), !dbg !42
  %n77 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0)), !dbg !42
  %76 = extractvalue [3 x i32] %74, 1, !dbg !42
  %n78 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32 %76), !dbg !42
  %n79 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0)), !dbg !42
  %77 = extractvalue [3 x i32] %74, 2, !dbg !42
  %n80 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32 %77), !dbg !42
  %n81 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @8, i32 0, i32 0)), !dbg !42
  %n82 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @9, i32 0, i32 0)), !dbg !42
  %n83 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0)), !dbg !42
  %78 = extractvalue [2 x %AA] %71, 1, !dbg !42
  %n84 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @1, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0)), !dbg !42
  %n85 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @3, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @2, i32 0, i32 0)), !dbg !42
  %79 = extractvalue %AA %78, 0, !dbg !42
  %n86 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32 %79), !dbg !42
  %n87 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0)), !dbg !42
  %n88 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @3, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @6, i32 0, i32 0)), !dbg !42
  %80 = extractvalue %AA %78, 1, !dbg !42
  %n89 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @7, i32 0, i32 0)), !dbg !42
  %81 = extractvalue [3 x i32] %80, 0, !dbg !42
  %n90 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32 %81), !dbg !42
  %n91 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0)), !dbg !42
  %82 = extractvalue [3 x i32] %80, 1, !dbg !42
  %n92 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32 %82), !dbg !42
  %n93 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0)), !dbg !42
  %83 = extractvalue [3 x i32] %80, 2, !dbg !42
  %n94 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32 %83), !dbg !42
  %n95 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @8, i32 0, i32 0)), !dbg !42
  %n96 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @9, i32 0, i32 0)), !dbg !42
  %n97 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @8, i32 0, i32 0)), !dbg !42
  %n98 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @8, i32 0, i32 0)), !dbg !42
  %n99 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @9, i32 0, i32 0)), !dbg !42
  %n100 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @10, i32 0, i32 0), i8 10), !dbg !43
  br label %ret, !dbg !44

ret:                                              ; preds = %entry
  ret void, !dbg !44
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
!38 = !DILocation(line: 19, column: 13, scope: !4)
!39 = !DILocation(line: 19, column: 25, scope: !4)
!40 = !DILocation(line: 20, column: 13, scope: !4)
!41 = !DILocation(line: 20, column: 25, scope: !4)
!42 = !DILocation(line: 21, column: 13, scope: !4)
!43 = !DILocation(line: 21, column: 25, scope: !4)
!44 = !DILocation(line: 22, column: 1, scope: !4)
