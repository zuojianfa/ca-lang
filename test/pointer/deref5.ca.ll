; ModuleID = 'deref5.ca'
source_filename = "deref5.ca"

@0 = private unnamed_addr constant [3 x i8] c"%p\00", align 1
@1 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@2 = private unnamed_addr constant [3 x i8] c"%p\00", align 1
@3 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@4 = private unnamed_addr constant [3 x i8] c"%p\00", align 1
@5 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@6 = private unnamed_addr constant [3 x i8] c"%p\00", align 1
@7 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@8 = private unnamed_addr constant [3 x i8] c"%p\00", align 1
@9 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@10 = private unnamed_addr constant [3 x i8] c"%p\00", align 1
@11 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@12 = private unnamed_addr constant [3 x i8] c"%p\00", align 1
@13 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@14 = private unnamed_addr constant [3 x i8] c"%p\00", align 1
@15 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@16 = private unnamed_addr constant [3 x i8] c"%p\00", align 1
@17 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@18 = private unnamed_addr constant [3 x i8] c"%p\00", align 1
@19 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@20 = private unnamed_addr constant [3 x i8] c"%p\00", align 1
@21 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@22 = private unnamed_addr constant [3 x i8] c"%p\00", align 1
@23 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@24 = private unnamed_addr constant [3 x i8] c"%p\00", align 1
@25 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@26 = private unnamed_addr constant [3 x i8] c"%p\00", align 1
@27 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@28 = private unnamed_addr constant [3 x i8] c"%p\00", align 1
@29 = private unnamed_addr constant [3 x i8] c"%c\00", align 1

declare i32 @printf(i8*, ...)

define void @main() {
entry:
  %a = alloca i32, align 4
  store volatile i32 1, i32* %a, align 4
  %pa = alloca i32*, align 8
  store volatile i32* %a, i32** %pa, align 8
  store volatile i32 2, i32* %a, align 4
  %load = load i32*, i32** %pa, align 8
  %n = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i32* %load)
  %n1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @1, i32 0, i32 0), i8 10)
  %v1 = load i32*, i32** %pa, align 8
  %pop = getelementptr i32, i32* %v1, i32 1
  %pb = alloca i32*, align 8
  store volatile i32* %pop, i32** %pb, align 8
  %load2 = load i32*, i32** %pa, align 8
  %n3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @2, i32 0, i32 0), i32* %load2)
  %n4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @3, i32 0, i32 0), i8 10)
  %load5 = load i32*, i32** %pb, align 8
  %n6 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32* %load5)
  %n7 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0), i8 10)
  %v18 = load i32*, i32** %pa, align 8
  %pop9 = getelementptr i32, i32* %v18, i32 2
  %n10 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @6, i32 0, i32 0), i32* %pop9)
  %n11 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @7, i32 0, i32 0), i8 10)
  %v112 = load i32*, i32** %pa, align 8
  %pop13 = getelementptr i32, i32* %v112, i32 1
  %pop14 = getelementptr i32, i32* %pop13, i32 2
  %pc = alloca i32*, align 8
  store volatile i32* %pop14, i32** %pc, align 8
  %load15 = load i32*, i32** %pc, align 8
  %n16 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @8, i32 0, i32 0), i32* %load15)
  %n17 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @9, i32 0, i32 0), i8 10)
  %v118 = load i32*, i32** %pa, align 8
  %pop19 = getelementptr i32, i32* %v118, i32 2
  %pop20 = getelementptr i32, i32* %pop19, i32 6
  %pd = alloca i32*, align 8
  store volatile i32* %pop20, i32** %pd, align 8
  %load21 = load i32*, i32** %pd, align 8
  %n22 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @10, i32 0, i32 0), i32* %load21)
  %n23 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @11, i32 0, i32 0), i8 10)
  %v124 = load i32*, i32** %pa, align 8
  %pop25 = getelementptr i32, i32* %v124, i32 2
  %pop26 = getelementptr i32, i32* %pop25, i32 6
  %pe = alloca i32*, align 8
  store volatile i32* %pop26, i32** %pe, align 8
  %load27 = load i32*, i32** %pe, align 8
  %n28 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @12, i32 0, i32 0), i32* %load27)
  %n29 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @13, i32 0, i32 0), i8 10)
  %v130 = load i32*, i32** %pa, align 8
  %pop31 = getelementptr i32, i32* %v130, i32 -1
  %pf0 = alloca i32*, align 8
  store volatile i32* %pop31, i32** %pf0, align 8
  %load32 = load i32*, i32** %pf0, align 8
  %n33 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @14, i32 0, i32 0), i32* %load32)
  %n34 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @15, i32 0, i32 0), i8 10)
  %b = alloca i64, align 8
  store volatile i64 3, i64* %b, align 4
  %load35 = load i32*, i32** %pa, align 8
  %n36 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @16, i32 0, i32 0), i32* %load35)
  %n37 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @17, i32 0, i32 0), i8 10)
  %v138 = load i32*, i32** %pa, align 8
  %load39 = load i64, i64* %b, align 4
  %neg = sub i64 0, %load39
  %pop40 = getelementptr i32, i32* %v138, i64 %neg
  %pf1 = alloca i32*, align 8
  store volatile i32* %pop40, i32** %pf1, align 8
  %load41 = load i32*, i32** %pf1, align 8
  %n42 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @18, i32 0, i32 0), i32* %load41)
  %n43 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @19, i32 0, i32 0), i8 10)
  %v144 = load i32*, i32** %pa, align 8
  %pop45 = getelementptr i32, i32* %v144, i64 -2
  %pf = alloca i32*, align 8
  store volatile i32* %pop45, i32** %pf, align 8
  %load46 = load i32*, i32** %pf, align 8
  %n47 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @20, i32 0, i32 0), i32* %load46)
  %n48 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @21, i32 0, i32 0), i8 10)
  %v149 = load i32*, i32** %pa, align 8
  %v2 = load i64, i64* %b, align 4
  %m = sub i64 0, %v2
  %pop50 = getelementptr i32, i32* %v149, i64 %m
  %pf11 = alloca i32*, align 8
  store volatile i32* %pop50, i32** %pf11, align 8
  %load51 = load i32*, i32** %pf11, align 8
  %n52 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @22, i32 0, i32 0), i32* %load51)
  %n53 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @23, i32 0, i32 0), i8 10)
  %v154 = load i32*, i32** %pa, align 8
  %v255 = load i64, i64* %b, align 4
  %m56 = sub i64 0, %v255
  %pop57 = getelementptr i32, i32* %v154, i64 %m56
  %pop58 = getelementptr i32, i32* %pop57, i32 1
  %pf2 = alloca i32*, align 8
  store volatile i32* %pop58, i32** %pf2, align 8
  %load59 = load i32*, i32** %pf2, align 8
  %n60 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @24, i32 0, i32 0), i32* %load59)
  %n61 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @25, i32 0, i32 0), i8 10)
  %v162 = load i32*, i32** %pa, align 8
  %pop63 = getelementptr i32, i32* %v162, i32 3
  %pop64 = getelementptr i32, i32* %pop63, i64 -1
  %pg = alloca i32*, align 8
  store volatile i32* %pop64, i32** %pg, align 8
  %load65 = load i32*, i32** %pg, align 8
  %n66 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @26, i32 0, i32 0), i32* %load65)
  %n67 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @27, i32 0, i32 0), i8 10)
  %v168 = load i32*, i32** %pa, align 8
  %pop69 = getelementptr i32, i32* %v168, i32 1
  %ph = alloca i32*, align 8
  store volatile i32* %pop69, i32** %ph, align 8
  %load70 = load i32*, i32** %ph, align 8
  %n71 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @28, i32 0, i32 0), i32* %load70)
  %n72 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @29, i32 0, i32 0), i8 10)
  br label %ret

ret:                                              ; preds = %entry
  ret void
}
