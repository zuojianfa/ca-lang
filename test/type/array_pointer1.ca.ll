; ModuleID = 'array_pointer1.ca'
source_filename = "array_pointer1.ca"

@0 = private unnamed_addr constant [3 x i8] c"%c\00", align 1

declare i32 @printf(i8*, ...)

define void @main() {
entry:
  %v92 = alloca [15 x [7 x [3 x [1 x i32]]]], align 4
  %v91 = alloca [15 x [7 x [3 x [1 x i32]]]], align 4
  %v90 = alloca [7 x [3 x [1 x i32]]]*, align 8
  %v89 = alloca [14 x [3 x [1 x i32]]*], align 8
  %v88 = alloca [3 x [1 x i32]]**, align 8
  %v87 = alloca [13 x [6 x [1 x i32]*]], align 8
  %v86 = alloca [6 x [1 x i32]*]*, align 8
  %v85 = alloca [12 x [1 x i32]**], align 8
  %v84 = alloca [1 x i32]***, align 8
  %v83 = alloca [11 x [5 x [2 x i32*]]], align 8
  %v82 = alloca [5 x [2 x i32*]]*, align 8
  %v81 = alloca [10 x [2 x i32*]*], align 8
  %v80 = alloca [2 x i32*]**, align 8
  %v79 = alloca [9 x [4 x i32**]], align 8
  %v78 = alloca [4 x i32**]*, align 8
  %v77 = alloca [8 x i32***], align 8
  %v76 = alloca i32****, align 8
  %v75 = alloca [7 x [3 x [1 x i32]]], align 4
  %v74 = alloca [3 x [1 x i32]]*, align 8
  %v73 = alloca [6 x [1 x i32]*], align 8
  %v72 = alloca [1 x i32]**, align 8
  %v71 = alloca [5 x [2 x i32*]], align 8
  %v70 = alloca [2 x i32*]*, align 8
  %v69 = alloca [4 x i32**], align 8
  %v68 = alloca i32***, align 8
  %v67 = alloca [3 x [1 x i32]], align 4
  %v66 = alloca [1 x i32]*, align 8
  %v65 = alloca [2 x i32*], align 8
  %v64 = alloca i32**, align 8
  %v63 = alloca [1 x i32], align 4
  %v62 = alloca i32*, align 8
  %v61 = alloca [15 x [7 x [3 x [1 x i32]]]], align 4
  %v60 = alloca [7 x [3 x [1 x i32]]]*, align 8
  %v59 = alloca [14 x [3 x [1 x i32]]*], align 8
  %v58 = alloca [3 x [1 x i32]]**, align 8
  %v57 = alloca [13 x [6 x [1 x i32]*]], align 8
  %v56 = alloca [6 x [1 x i32]*]*, align 8
  %v55 = alloca [12 x [1 x i32]**], align 8
  %v54 = alloca [1 x i32]***, align 8
  %v53 = alloca [11 x [5 x [2 x i32*]]], align 8
  %v52 = alloca [5 x [2 x i32*]]*, align 8
  %v51 = alloca [10 x [2 x i32*]*], align 8
  %v50 = alloca [2 x i32*]**, align 8
  %v49 = alloca [9 x [4 x i32**]], align 8
  %v48 = alloca [4 x i32**]*, align 8
  %v47 = alloca [8 x i32***], align 8
  %v46 = alloca i32****, align 8
  %v45 = alloca [7 x [3 x [1 x i32]]], align 4
  %v44 = alloca [3 x [1 x i32]]*, align 8
  %v43 = alloca [6 x [1 x i32]*], align 8
  %v42 = alloca [1 x i32]**, align 8
  %v41 = alloca [5 x [2 x i32*]], align 8
  %v40 = alloca [2 x i32*]*, align 8
  %v39 = alloca [4 x i32**], align 8
  %v38 = alloca i32***, align 8
  %v37 = alloca [3 x [1 x i32]], align 4
  %v36 = alloca [1 x i32]*, align 8
  %v35 = alloca [2 x i32*], align 8
  %v34 = alloca i32**, align 8
  %v33 = alloca [1 x i32], align 4
  %v32 = alloca i32*, align 8
  %v31 = alloca [15 x [7 x [3 x [1 x i32]]]], align 4
  %v30 = alloca [7 x [3 x [1 x i32]]]*, align 8
  %v29 = alloca [14 x [3 x [1 x i32]]*], align 8
  %v28 = alloca [3 x [1 x i32]]**, align 8
  %v27 = alloca [13 x [6 x [1 x i32]*]], align 8
  %v26 = alloca [6 x [1 x i32]*]*, align 8
  %v25 = alloca [12 x [1 x i32]**], align 8
  %v24 = alloca [1 x i32]***, align 8
  %v23 = alloca [11 x [5 x [2 x i32*]]], align 8
  %v22 = alloca [5 x [2 x i32*]]*, align 8
  %v21 = alloca [10 x [2 x i32*]*], align 8
  %v20 = alloca [2 x i32*]**, align 8
  %v19 = alloca [9 x [4 x i32**]], align 8
  %v18 = alloca [4 x i32**]*, align 8
  %v17 = alloca [8 x i32***], align 8
  %v16 = alloca i32****, align 8
  %v15 = alloca [7 x [3 x [1 x i32]]], align 4
  %v14 = alloca [3 x [1 x i32]]*, align 8
  %v13 = alloca [6 x [1 x i32]*], align 8
  %v12 = alloca [1 x i32]**, align 8
  %v11 = alloca [5 x [2 x i32*]], align 8
  %v10 = alloca [2 x i32*]*, align 8
  %v9 = alloca [4 x i32**], align 8
  %v8 = alloca i32***, align 8
  %v7 = alloca [3 x [1 x i32]], align 4
  %v6 = alloca [1 x i32]*, align 8
  %v5 = alloca [2 x i32*], align 8
  %v4 = alloca i32**, align 8
  %v3 = alloca [1 x i32], align 4
  %v2 = alloca i32*, align 8
  %v1 = alloca i32, align 4
  store volatile i32 0, i32* %v1, align 4
  %0 = bitcast i32** %v2 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %0, i8 0, i64 8, i1 false)
  %1 = bitcast [1 x i32]* %v3 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %1, i8 0, i64 4, i1 false)
  %2 = bitcast i32*** %v4 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %2, i8 0, i64 8, i1 false)
  %3 = bitcast [2 x i32*]* %v5 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %3, i8 0, i64 16, i1 false)
  %4 = bitcast [1 x i32]** %v6 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %4, i8 0, i64 8, i1 false)
  %5 = bitcast [3 x [1 x i32]]* %v7 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %5, i8 0, i64 12, i1 false)
  %6 = bitcast i32**** %v8 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %6, i8 0, i64 8, i1 false)
  %7 = bitcast [4 x i32**]* %v9 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %7, i8 0, i64 32, i1 false)
  %8 = bitcast [2 x i32*]** %v10 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %8, i8 0, i64 8, i1 false)
  %9 = bitcast [5 x [2 x i32*]]* %v11 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %9, i8 0, i64 80, i1 false)
  %10 = bitcast [1 x i32]*** %v12 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %10, i8 0, i64 8, i1 false)
  %11 = bitcast [6 x [1 x i32]*]* %v13 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %11, i8 0, i64 48, i1 false)
  %12 = bitcast [3 x [1 x i32]]** %v14 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %12, i8 0, i64 8, i1 false)
  %13 = bitcast [7 x [3 x [1 x i32]]]* %v15 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %13, i8 0, i64 84, i1 false)
  %14 = bitcast i32***** %v16 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %14, i8 0, i64 8, i1 false)
  %15 = bitcast [8 x i32***]* %v17 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %15, i8 0, i64 64, i1 false)
  %16 = bitcast [4 x i32**]** %v18 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %16, i8 0, i64 8, i1 false)
  %17 = bitcast [9 x [4 x i32**]]* %v19 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %17, i8 0, i64 288, i1 false)
  %18 = bitcast [2 x i32*]*** %v20 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %18, i8 0, i64 8, i1 false)
  %19 = bitcast [10 x [2 x i32*]*]* %v21 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %19, i8 0, i64 80, i1 false)
  %20 = bitcast [5 x [2 x i32*]]** %v22 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %20, i8 0, i64 8, i1 false)
  %21 = bitcast [11 x [5 x [2 x i32*]]]* %v23 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %21, i8 0, i64 880, i1 false)
  %22 = bitcast [1 x i32]**** %v24 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %22, i8 0, i64 8, i1 false)
  %23 = bitcast [12 x [1 x i32]**]* %v25 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %23, i8 0, i64 96, i1 false)
  %24 = bitcast [6 x [1 x i32]*]** %v26 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %24, i8 0, i64 8, i1 false)
  %25 = bitcast [13 x [6 x [1 x i32]*]]* %v27 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %25, i8 0, i64 624, i1 false)
  %26 = bitcast [3 x [1 x i32]]*** %v28 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %26, i8 0, i64 8, i1 false)
  %27 = bitcast [14 x [3 x [1 x i32]]*]* %v29 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %27, i8 0, i64 112, i1 false)
  %28 = bitcast [7 x [3 x [1 x i32]]]** %v30 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %28, i8 0, i64 8, i1 false)
  %29 = bitcast [15 x [7 x [3 x [1 x i32]]]]* %v31 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %29, i8 0, i64 1260, i1 false)
  %30 = bitcast i32** %v32 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %30, i8 0, i64 8, i1 false)
  %31 = bitcast [1 x i32]* %v33 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %31, i8 0, i64 4, i1 false)
  %32 = bitcast i32*** %v34 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %32, i8 0, i64 8, i1 false)
  %33 = bitcast [2 x i32*]* %v35 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %33, i8 0, i64 16, i1 false)
  %34 = bitcast [1 x i32]** %v36 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %34, i8 0, i64 8, i1 false)
  %35 = bitcast [3 x [1 x i32]]* %v37 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %35, i8 0, i64 12, i1 false)
  %36 = bitcast i32**** %v38 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %36, i8 0, i64 8, i1 false)
  %37 = bitcast [4 x i32**]* %v39 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %37, i8 0, i64 32, i1 false)
  %38 = bitcast [2 x i32*]** %v40 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %38, i8 0, i64 8, i1 false)
  %39 = bitcast [5 x [2 x i32*]]* %v41 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %39, i8 0, i64 80, i1 false)
  %40 = bitcast [1 x i32]*** %v42 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %40, i8 0, i64 8, i1 false)
  %41 = bitcast [6 x [1 x i32]*]* %v43 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %41, i8 0, i64 48, i1 false)
  %42 = bitcast [3 x [1 x i32]]** %v44 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %42, i8 0, i64 8, i1 false)
  %43 = bitcast [7 x [3 x [1 x i32]]]* %v45 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %43, i8 0, i64 84, i1 false)
  %44 = bitcast i32***** %v46 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %44, i8 0, i64 8, i1 false)
  %45 = bitcast [8 x i32***]* %v47 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %45, i8 0, i64 64, i1 false)
  %46 = bitcast [4 x i32**]** %v48 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %46, i8 0, i64 8, i1 false)
  %47 = bitcast [9 x [4 x i32**]]* %v49 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %47, i8 0, i64 288, i1 false)
  %48 = bitcast [2 x i32*]*** %v50 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %48, i8 0, i64 8, i1 false)
  %49 = bitcast [10 x [2 x i32*]*]* %v51 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %49, i8 0, i64 80, i1 false)
  %50 = bitcast [5 x [2 x i32*]]** %v52 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %50, i8 0, i64 8, i1 false)
  %51 = bitcast [11 x [5 x [2 x i32*]]]* %v53 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %51, i8 0, i64 880, i1 false)
  %52 = bitcast [1 x i32]**** %v54 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %52, i8 0, i64 8, i1 false)
  %53 = bitcast [12 x [1 x i32]**]* %v55 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %53, i8 0, i64 96, i1 false)
  %54 = bitcast [6 x [1 x i32]*]** %v56 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %54, i8 0, i64 8, i1 false)
  %55 = bitcast [13 x [6 x [1 x i32]*]]* %v57 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %55, i8 0, i64 624, i1 false)
  %56 = bitcast [3 x [1 x i32]]*** %v58 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %56, i8 0, i64 8, i1 false)
  %57 = bitcast [14 x [3 x [1 x i32]]*]* %v59 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %57, i8 0, i64 112, i1 false)
  %58 = bitcast [7 x [3 x [1 x i32]]]** %v60 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %58, i8 0, i64 8, i1 false)
  %59 = bitcast [15 x [7 x [3 x [1 x i32]]]]* %v61 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %59, i8 0, i64 1260, i1 false)
  %60 = bitcast i32** %v62 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %60, i8 0, i64 8, i1 false)
  %61 = bitcast [1 x i32]* %v63 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %61, i8 0, i64 4, i1 false)
  %62 = bitcast i32*** %v64 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %62, i8 0, i64 8, i1 false)
  %63 = bitcast [2 x i32*]* %v65 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %63, i8 0, i64 16, i1 false)
  %64 = bitcast [1 x i32]** %v66 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %64, i8 0, i64 8, i1 false)
  %65 = bitcast [3 x [1 x i32]]* %v67 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %65, i8 0, i64 12, i1 false)
  %66 = bitcast i32**** %v68 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %66, i8 0, i64 8, i1 false)
  %67 = bitcast [4 x i32**]* %v69 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %67, i8 0, i64 32, i1 false)
  %68 = bitcast [2 x i32*]** %v70 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %68, i8 0, i64 8, i1 false)
  %69 = bitcast [5 x [2 x i32*]]* %v71 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %69, i8 0, i64 80, i1 false)
  %70 = bitcast [1 x i32]*** %v72 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %70, i8 0, i64 8, i1 false)
  %71 = bitcast [6 x [1 x i32]*]* %v73 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %71, i8 0, i64 48, i1 false)
  %72 = bitcast [3 x [1 x i32]]** %v74 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %72, i8 0, i64 8, i1 false)
  %73 = bitcast [7 x [3 x [1 x i32]]]* %v75 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %73, i8 0, i64 84, i1 false)
  %74 = bitcast i32***** %v76 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %74, i8 0, i64 8, i1 false)
  %75 = bitcast [8 x i32***]* %v77 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %75, i8 0, i64 64, i1 false)
  %76 = bitcast [4 x i32**]** %v78 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %76, i8 0, i64 8, i1 false)
  %77 = bitcast [9 x [4 x i32**]]* %v79 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %77, i8 0, i64 288, i1 false)
  %78 = bitcast [2 x i32*]*** %v80 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %78, i8 0, i64 8, i1 false)
  %79 = bitcast [10 x [2 x i32*]*]* %v81 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %79, i8 0, i64 80, i1 false)
  %80 = bitcast [5 x [2 x i32*]]** %v82 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %80, i8 0, i64 8, i1 false)
  %81 = bitcast [11 x [5 x [2 x i32*]]]* %v83 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %81, i8 0, i64 880, i1 false)
  %82 = bitcast [1 x i32]**** %v84 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %82, i8 0, i64 8, i1 false)
  %83 = bitcast [12 x [1 x i32]**]* %v85 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %83, i8 0, i64 96, i1 false)
  %84 = bitcast [6 x [1 x i32]*]** %v86 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %84, i8 0, i64 8, i1 false)
  %85 = bitcast [13 x [6 x [1 x i32]*]]* %v87 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %85, i8 0, i64 624, i1 false)
  %86 = bitcast [3 x [1 x i32]]*** %v88 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %86, i8 0, i64 8, i1 false)
  %87 = bitcast [14 x [3 x [1 x i32]]*]* %v89 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %87, i8 0, i64 112, i1 false)
  %88 = bitcast [7 x [3 x [1 x i32]]]** %v90 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %88, i8 0, i64 8, i1 false)
  %89 = bitcast [15 x [7 x [3 x [1 x i32]]]]* %v91 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %89, i8 0, i64 1260, i1 false)
  %90 = bitcast [15 x [7 x [3 x [1 x i32]]]]* %v92 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %90, i8 0, i64 1260, i1 false)
  %n = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 103)
  %n1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 111)
  %n2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 111)
  %n3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 100)
  %n4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 10)
  br label %ret

ret:                                              ; preds = %entry
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #0

attributes #0 = { argmemonly nofree nosync nounwind willreturn writeonly }
