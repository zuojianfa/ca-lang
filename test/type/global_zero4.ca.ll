; ModuleID = 'global_zero4.ca'
source_filename = "global_zero4.ca"

%SS = type { [3 x float], [4 x double], i32, i64 }
%S1 = type { [33 x %SS], [2 x i32] }
%S2 = type { [2 x %S1], [3 x double]* }
%S3 = type { %S2***, [9 x %S2***], [3 x i1***] }
%S4 = type { [4 x [3 x [2 x %S1]]], [8 x [7 x [6 x [3 x double]*]]*] }

@aa = internal global [3 x %SS] zeroinitializer, align 4
@a1 = internal global [33 x %S1] zeroinitializer, align 4
@a2 = internal global [2 x %S2] zeroinitializer, align 4
@a3 = internal global %S3*** zeroinitializer, align 4
@a4 = internal global [9 x %S3***] zeroinitializer, align 4
@a5 = internal global [3 x %S2***] zeroinitializer, align 4
@a6 = internal global [6 x [2 x [3 x %S4]]] zeroinitializer, align 4
@a71 = internal global [2 x [3 x %S4]] zeroinitializer, align 4
@a7 = internal global [33 x [6 x [2 x [3 x %S4]]]] zeroinitializer, align 4
@a8 = internal global [1 x [33 x [6 x [2 x [3 x %S4]]]]] zeroinitializer, align 4
@a9 = internal global [7 x [1 x [33 x [6 x [2 x [3 x %S4]]]]]] zeroinitializer, align 4

declare i32 @printf(i8*, ...)
