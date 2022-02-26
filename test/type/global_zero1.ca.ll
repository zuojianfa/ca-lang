; ModuleID = 'global_zero1.ca'
source_filename = "global_zero1.ca"

%AA = type { float, double, i32, i64 }
%A1 = type { %AA, i32 }
%A2 = type { %A1, double }
%A3 = type { %A2, i1 }

@aa = internal global %AA zeroinitializer, align 4
@a1 = internal global %A1 zeroinitializer, align 4
@a2 = internal global %A2 zeroinitializer, align 4
@a3 = internal global %A3 zeroinitializer, align 4
@0 = private unnamed_addr constant [6 x i8] c"good\0A\00", align 1
@1 = private unnamed_addr constant [3 x i8] c"%s\00", align 1

declare i32 @printf(i8*, ...)

define void @main() {
entry:
  %n = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @1, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @0, i32 0, i32 0))
  br label %ret

ret:                                              ; preds = %entry
  ret void
}
