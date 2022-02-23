; ModuleID = 'global_zero2.ca'
source_filename = "global_zero2.ca"

@aa = internal global { [3 x float], [4 x double], i32, i64 } zeroinitializer, align 4
@a1 = internal global { [33 x { [3 x float], [4 x double], i32, i64 }], [2 x i32] } zeroinitializer, align 4
@a2 = internal global { [2 x { [33 x { [3 x float], [4 x double], i32, i64 }], [2 x i32] }], [3 x double]* } zeroinitializer, align 4
@a3 = internal global { { [2 x { [33 x { [3 x float], [4 x double], i32, i64 }], [2 x i32] }], [3 x double]* }***, [9 x { [2 x { [33 x { [3 x float], [4 x double], i32, i64 }], [2 x i32] }], [3 x double]* }***], [3 x i1***] } zeroinitializer, align 4

declare i32 @printf(i8*, ...)
