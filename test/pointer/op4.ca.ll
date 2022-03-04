; ModuleID = 'op4.ca'
source_filename = "op4.ca"

@0 = private unnamed_addr constant [3 x i8] c"%p\00", align 1
@1 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@2 = private unnamed_addr constant [3 x i8] c"%p\00", align 1
@3 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@4 = private unnamed_addr constant [3 x i8] c"%p\00", align 1
@5 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@6 = private unnamed_addr constant [3 x i8] c"%p\00", align 1
@7 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@8 = private unnamed_addr constant [2 x i8] c"[\00", align 1
@9 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@10 = private unnamed_addr constant [3 x i8] c", \00", align 1
@11 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@12 = private unnamed_addr constant [3 x i8] c", \00", align 1
@13 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@14 = private unnamed_addr constant [3 x i8] c", \00", align 1
@15 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@16 = private unnamed_addr constant [2 x i8] c"]\00", align 1
@17 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@18 = private unnamed_addr constant [3 x i8] c"%p\00", align 1
@19 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@20 = private unnamed_addr constant [3 x i8] c"%p\00", align 1
@21 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@22 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@23 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@24 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@25 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@26 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@27 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@28 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@29 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@30 = private unnamed_addr constant [3 x i8] c"%p\00", align 1
@31 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@32 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@33 = private unnamed_addr constant [3 x i8] c"%c\00", align 1

declare i32 @printf(i8*, ...)

declare i32 @strlen(i8*)

define void @main() {
entry:
  %0 = alloca [4 x i32], align 4
  %1 = getelementptr [4 x i32], [4 x i32]* %0, i32 0, i64 0
  store volatile i32 1, i32* %1, align 4
  %2 = getelementptr [4 x i32], [4 x i32]* %0, i32 0, i64 1
  store volatile i32 1, i32* %2, align 4
  %3 = getelementptr [4 x i32], [4 x i32]* %0, i32 0, i64 2
  store volatile i32 1, i32* %3, align 4
  %4 = getelementptr [4 x i32], [4 x i32]* %0, i32 0, i64 3
  store volatile i32 1, i32* %4, align 4
  %a = alloca [4 x i32], align 4
  %5 = bitcast [4 x i32]* %a to i8*
  %6 = bitcast [4 x i32]* %0 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %5, i8* align 4 %6, i64 16, i1 false)
  %7 = bitcast [4 x i32]* %a to i32*
  %pa = alloca i32*, align 8
  store volatile i32* %7, i32** %pa, align 8
  %ppa = alloca i32**, align 8
  store volatile i32** %pa, i32*** %ppa, align 8
  %load = load i32*, i32** %pa, align 8
  %n = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i32* %load)
  %n1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @1, i32 0, i32 0), i8 10)
  %v1 = load i32*, i32** %pa, align 8
  %pop = getelementptr i32, i32* %v1, i64 -1
  store volatile i32* %pop, i32** %pa, align 8
  %load2 = load i32*, i32** %pa, align 8
  %n3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @2, i32 0, i32 0), i32* %load2)
  %n4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @3, i32 0, i32 0), i8 10)
  %load5 = load i32*, i32** %pa, align 8
  %n6 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32* %load5)
  %n7 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0), i8 10)
  %v18 = load i32*, i32** %pa, align 8
  %pop9 = getelementptr i32, i32* %v18, i32 1
  %pop10 = getelementptr i32, i32* %pop9, i32 2
  store volatile i32 4, i32* %pop10, align 4
  %load11 = load i32*, i32** %pa, align 8
  %n12 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @6, i32 0, i32 0), i32* %load11)
  %n13 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @7, i32 0, i32 0), i8 10)
  %v114 = load i32*, i32** %pa, align 8
  %pop15 = getelementptr i32, i32* %v114, i32 3
  store volatile i32 4, i32* %pop15, align 4
  %load16 = load [4 x i32], [4 x i32]* %a, align 4
  %n17 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @8, i32 0, i32 0))
  %8 = extractvalue [4 x i32] %load16, 0
  %n18 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @9, i32 0, i32 0), i32 %8)
  %n19 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @10, i32 0, i32 0))
  %9 = extractvalue [4 x i32] %load16, 1
  %n20 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @11, i32 0, i32 0), i32 %9)
  %n21 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @12, i32 0, i32 0))
  %10 = extractvalue [4 x i32] %load16, 2
  %n22 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @13, i32 0, i32 0), i32 %10)
  %n23 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @14, i32 0, i32 0))
  %11 = extractvalue [4 x i32] %load16, 3
  %n24 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @15, i32 0, i32 0), i32 %11)
  %n25 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @16, i32 0, i32 0))
  %n26 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @17, i32 0, i32 0), i8 10)
  %load27 = load i32*, i32** %pa, align 8
  %n28 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @18, i32 0, i32 0), i32* %load27)
  %n29 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @19, i32 0, i32 0), i8 10)
  %load30 = load i32*, i32** %pa, align 8
  %n31 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @20, i32 0, i32 0), i32* %load30)
  %n32 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @21, i32 0, i32 0), i8 10)
  %v133 = load i32*, i32** %pa, align 8
  %pop34 = getelementptr i32, i32* %v133, i32 1
  %deref = load i32, i32* %pop34, align 4
  %c = alloca i32, align 4
  store volatile i32 %deref, i32* %c, align 4
  %v135 = load i32*, i32** %pa, align 8
  %pop36 = getelementptr i32, i32* %v135, i32 1
  %deref37 = load i32, i32* %pop36, align 4
  %n38 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @22, i32 0, i32 0), i32 %deref37)
  %n39 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @23, i32 0, i32 0), i8 10)
  %load40 = load i32, i32* %c, align 4
  %n41 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @24, i32 0, i32 0), i32 %load40)
  %n42 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @25, i32 0, i32 0), i8 10)
  %v143 = load i32*, i32** %pa, align 8
  %pop44 = getelementptr i32, i32* %v143, i32 1
  %pop45 = getelementptr i32, i32* %pop44, i32 2
  %deref46 = load i32, i32* %pop45, align 4
  %d = alloca i32, align 4
  store volatile i32 %deref46, i32* %d, align 4
  %v147 = load i32*, i32** %pa, align 8
  %pop48 = getelementptr i32, i32* %v147, i32 1
  %pop49 = getelementptr i32, i32* %pop48, i32 2
  %deref50 = load i32, i32* %pop49, align 4
  %n51 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @26, i32 0, i32 0), i32 %deref50)
  %n52 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @27, i32 0, i32 0), i8 10)
  %load53 = load i32, i32* %d, align 4
  %n54 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @28, i32 0, i32 0), i32 %load53)
  %n55 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @29, i32 0, i32 0), i8 10)
  %deref56 = load i32**, i32*** %ppa, align 8
  %tmpexpr = load i32*, i32** %deref56, align 8
  %e = alloca i32*, align 8
  store volatile i32* %tmpexpr, i32** %e, align 8
  %deref57 = load i32**, i32*** %ppa, align 8
  %load58 = load i32*, i32** %deref57, align 8
  %n59 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @30, i32 0, i32 0), i32* %load58)
  %n60 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @31, i32 0, i32 0), i8 10)
  %deref61 = load i32**, i32*** %ppa, align 8
  %v162 = load i32*, i32** %deref61, align 8
  %pop63 = getelementptr i32, i32* %v162, i32 1
  %deref64 = load i32, i32* %pop63, align 4
  %f = alloca i32, align 4
  store volatile i32 %deref64, i32* %f, align 4
  %load65 = load i32, i32* %f, align 4
  %n66 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @32, i32 0, i32 0), i32 %load65)
  %n67 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @33, i32 0, i32 0), i8 10)
  br label %ret

ret:                                              ; preds = %entry
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #0

attributes #0 = { argmemonly nofree nosync nounwind willreturn }
