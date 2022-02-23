; ModuleID = 'global_zero4.ca'
source_filename = "global_zero4.ca"

@aa = internal global [3 x { [3 x float], [4 x double], i32, i64 }] zeroinitializer, align 4
@a1 = internal global [33 x { [33 x { [3 x float], [4 x double], i32, i64 }], [2 x i32] }] zeroinitializer, align 4
@a2 = internal global [2 x { [2 x { [33 x { [3 x float], [4 x double], i32, i64 }], [2 x i32] }], [3 x double]* }] zeroinitializer, align 4
@a3 = internal global { { [2 x { [33 x { [3 x float], [4 x double], i32, i64 }], [2 x i32] }], [3 x double]* }***, [9 x { [2 x { [33 x { [3 x float], [4 x double], i32, i64 }], [2 x i32] }], [3 x double]* }***], [3 x i1***] }*** zeroinitializer, align 4
@a4 = internal global [9 x { { [2 x { [33 x { [3 x float], [4 x double], i32, i64 }], [2 x i32] }], [3 x double]* }***, [9 x { [2 x { [33 x { [3 x float], [4 x double], i32, i64 }], [2 x i32] }], [3 x double]* }***], [3 x i1***] }***] zeroinitializer, align 4
@a5 = internal global [3 x { [2 x { [33 x { [3 x float], [4 x double], i32, i64 }], [2 x i32] }], [3 x double]* }***] zeroinitializer, align 4
@a6 = internal global [6 x [2 x [3 x { [4 x [3 x [2 x { [33 x { [3 x float], [4 x double], i32, i64 }], [2 x i32] }]]], [8 x [7 x [6 x [3 x double]*]]*] }]]] zeroinitializer, align 4
@a71 = internal global [2 x [3 x { [4 x [3 x [2 x { [33 x { [3 x float], [4 x double], i32, i64 }], [2 x i32] }]]], [8 x [7 x [6 x [3 x double]*]]*] }]] zeroinitializer, align 4
@a7 = internal global [33 x [6 x [2 x [3 x { [4 x [3 x [2 x { [33 x { [3 x float], [4 x double], i32, i64 }], [2 x i32] }]]], [8 x [7 x [6 x [3 x double]*]]*] }]]]] zeroinitializer, align 4
@a8 = internal global [1 x [33 x [6 x [2 x [3 x { [4 x [3 x [2 x { [33 x { [3 x float], [4 x double], i32, i64 }], [2 x i32] }]]], [8 x [7 x [6 x [3 x double]*]]*] }]]]]] zeroinitializer, align 4
@a9 = internal global [7 x [1 x [33 x [6 x [2 x [3 x { [4 x [3 x [2 x { [33 x { [3 x float], [4 x double], i32, i64 }], [2 x i32] }]]], [8 x [7 x [6 x [3 x double]*]]*] }]]]]]] zeroinitializer, align 4

declare i32 @printf(i8*, ...)
