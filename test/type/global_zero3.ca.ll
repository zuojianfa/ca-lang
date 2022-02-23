; ModuleID = 'global_zero3.ca'
source_filename = "global_zero3.ca"

@aa = internal global [3 x float] zeroinitializer, align 4
@a1 = internal global [33 x [3 x float]] zeroinitializer, align 4
@a2 = internal global [2 x [33 x [3 x float]]] zeroinitializer, align 4
@a3 = internal global [2 x [33 x [3 x float]]]*** zeroinitializer, align 4
@a4 = internal global [9 x [2 x [33 x [3 x float]]]***] zeroinitializer, align 4
@a5 = internal global [3 x i1***] zeroinitializer, align 4
@a6 = internal global [6 x [2 x [3 x [9 x [2 x [33 x [3 x float]]]***]]]] zeroinitializer, align 4
@a7 = internal global [33 x [6 x [2 x [3 x [9 x [2 x [33 x [3 x float]]]***]]]]] zeroinitializer, align 4
@a8 = internal global [1 x [33 x [6 x [2 x [3 x [9 x [2 x [33 x [3 x float]]]***]]]]]] zeroinitializer, align 4
@a9 = internal global [7 x [1 x [33 x [6 x [2 x [3 x [9 x [2 x [33 x [3 x float]]]***]]]]]]] zeroinitializer, align 4

declare i32 @printf(i8*, ...)
