; ModuleID = 'struct_use1.ca'
source_filename = "struct_use1.ca"

@0 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@1 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@2 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@3 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@4 = private unnamed_addr constant [3 x i8] c"%c\00", align 1

declare i32 @printf(i8*, ...)

define void @func1() {
entry:
  br label %ret

ret:                                              ; preds = %entry
  ret void
}

define void @main() {
entry:
  call void @func1()
  %v0 = alloca {}, align 8
  %0 = bitcast {}* %v0 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %0, i8 0, i64 0, i1 false)
  %v1 = alloca {}, align 8
  %1 = bitcast {}* %v1 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %1, i8 0, i64 0, i1 false)
  %v2 = alloca { i32, i64, i32, float, double, i1, i8, i8 }, align 8
  %2 = bitcast { i32, i64, i32, float, double, i1, i8, i8 }* %v2 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %2, i8 0, i64 40, i1 false)
  %v3 = alloca { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }, align 8
  %3 = bitcast { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }* %v3 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %3, i8 0, i64 40, i1 false)
  %v4 = alloca { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }, align 8
  %4 = bitcast { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }* %v4 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %4, i8 0, i64 64, i1 false)
  %v5 = alloca { { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }, { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }*, { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }**, { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }***, { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }**** }, align 8
  %5 = bitcast { { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }, { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }*, { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }**, { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }***, { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }**** }* %v5 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %5, i8 0, i64 96, i1 false)
  %v6 = alloca { { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }, { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }*, { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }**, { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }***, { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }**** }, align 8
  %6 = bitcast { { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }, { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }*, { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }**, { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }***, { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }**** }* %v6 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %6, i8 0, i64 96, i1 false)
  %v7 = alloca { { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }, { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }*, { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }**, { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }***, { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }**** }*, align 8
  %7 = bitcast { { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }, { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }*, { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }**, { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }***, { i32, i32*, i32**, i32***, {}*, {}**, { {}, {}, { i32, i64, i32, float, double, i1, i8, i8 } }*, { i32, i64, i32, float, double, i1, i8, i8 }** }**** }** %v7 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %7, i8 0, i64 8, i1 false)
  %v8 = alloca [6 x [5 x [4 x [3 x { i32, i64, i32, float, double, i1, i8, i8 }]*]*]*]**, align 8
  %8 = bitcast [6 x [5 x [4 x [3 x { i32, i64, i32, float, double, i1, i8, i8 }]*]*]*]*** %v8 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %8, i8 0, i64 8, i1 false)
  %n = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i8 103)
  %n1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @1, i32 0, i32 0), i8 111)
  %n2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @2, i32 0, i32 0), i8 111)
  %n3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @3, i32 0, i32 0), i8 100)
  %n4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i8 10)
  br label %ret

ret:                                              ; preds = %entry
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #0

attributes #0 = { argmemonly nofree nosync nounwind willreturn writeonly }
