; ModuleID = 'global_zero2.ca'
source_filename = "global_zero2.ca"

%AA = type { [3 x float], [4 x double], i32, i64 }
%A1 = type { [33 x %AA], [2 x i32] }
%A2 = type { [2 x %A1], [3 x double]* }
%A3 = type { %A2***, [9 x %A2***], [3 x i1***] }

@aa = internal global %AA zeroinitializer, align 4
@a1 = internal global %A1 zeroinitializer, align 4
@a2 = internal global %A2 zeroinitializer, align 4
@a3 = internal global %A3 zeroinitializer, align 4

declare i32 @printf(i8*, ...)
