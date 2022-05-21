; ModuleID = 'deref6.ca'
source_filename = "deref6.ca"

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

declare i32 @printf(i8*, ...)

define void @main() {
entry:
  %pc = alloca i32*, align 8
  %pb = alloca i32*, align 8
  %pa = alloca i32*, align 8
  %a = alloca i32, align 4
  store volatile i32 2, i32* %a, align 4
  store volatile i32* %a, i32** %pa, align 8
  %load = load i32*, i32** %pa, align 8
  %n = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0), i32* %load)
  %n1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @1, i32 0, i32 0), i8 10)
  %v1 = load i32*, i32** %pa, align 8
  %pop = getelementptr i32, i32* %v1, i32 1
  store volatile i32* %pop, i32** %pb, align 8
  %load2 = load i32*, i32** %pb, align 8
  %n3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @2, i32 0, i32 0), i32* %load2)
  %n4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @3, i32 0, i32 0), i8 10)
  %v15 = load i32*, i32** %pa, align 8
  %pop6 = getelementptr i32, i32* %v15, i32 1
  store volatile i32* %pop6, i32** %pb, align 8
  %load7 = load i32*, i32** %pb, align 8
  %n8 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @4, i32 0, i32 0), i32* %load7)
  %n9 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @5, i32 0, i32 0), i8 10)
  %v110 = load i32*, i32** %pa, align 8
  %pop11 = getelementptr i32, i32* %v110, i32 1
  store volatile i32* %pop11, i32** %pa, align 8
  %load12 = load i32*, i32** %pa, align 8
  %n13 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @6, i32 0, i32 0), i32* %load12)
  %n14 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @7, i32 0, i32 0), i8 10)
  %v115 = load i32*, i32** %pa, align 8
  %pop16 = getelementptr i32, i32* %v115, i32 2
  store volatile i32* %pop16, i32** %pb, align 8
  %load17 = load i32*, i32** %pb, align 8
  %n18 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @8, i32 0, i32 0), i32* %load17)
  %n19 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @9, i32 0, i32 0), i8 10)
  %v120 = load i32*, i32** %pb, align 8
  %pop21 = getelementptr i32, i32* %v120, i64 -2
  store volatile i32* %pop21, i32** %pb, align 8
  %load22 = load i32*, i32** %pb, align 8
  %n23 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @10, i32 0, i32 0), i32* %load22)
  %n24 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @11, i32 0, i32 0), i8 10)
  %v125 = load i32*, i32** %pa, align 8
  %pop26 = getelementptr i32, i32* %v125, i32 -2
  store volatile i32* %pop26, i32** %pb, align 8
  %load27 = load i32*, i32** %pb, align 8
  %n28 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @12, i32 0, i32 0), i32* %load27)
  %n29 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @13, i32 0, i32 0), i8 10)
  %v130 = load i32*, i32** %pa, align 8
  %pop31 = getelementptr i32, i32* %v130, i32 2
  %pop32 = getelementptr i32, i32* %pop31, i64 -1
  store volatile i32* %pop32, i32** %pc, align 8
  %load33 = load i32*, i32** %pc, align 8
  %n34 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @14, i32 0, i32 0), i32* %load33)
  %n35 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @15, i32 0, i32 0), i8 10)
  br label %ret

ret:                                              ; preds = %entry
  ret void
}
