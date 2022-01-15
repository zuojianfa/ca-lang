; ModuleID = 'array_var.c'
source_filename = "array_var.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @func1() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca [2 x [3 x i32]], align 16
  store i32 1, i32* %1, align 4
  store i32 2, i32* %2, align 4
  store i32 3, i32* %3, align 4
  store i32 4, i32* %4, align 4
  store i32 5, i32* %5, align 4
  store i32 6, i32* %6, align 4
  %8 = getelementptr inbounds [2 x [3 x i32]], [2 x [3 x i32]]* %7, i64 0, i64 0
  %9 = getelementptr inbounds [3 x i32], [3 x i32]* %8, i64 0, i64 0
  %10 = load i32, i32* %1, align 4
  store i32 %10, i32* %9, align 4
  %11 = getelementptr inbounds i32, i32* %9, i64 1
  %12 = load i32, i32* %2, align 4
  store i32 %12, i32* %11, align 4
  %13 = getelementptr inbounds i32, i32* %11, i64 1
  %14 = load i32, i32* %3, align 4
  store i32 %14, i32* %13, align 4
  %15 = getelementptr inbounds [3 x i32], [3 x i32]* %8, i64 1
  %16 = getelementptr inbounds [3 x i32], [3 x i32]* %15, i64 0, i64 0
  %17 = load i32, i32* %4, align 4
  store i32 %17, i32* %16, align 4
  %18 = getelementptr inbounds i32, i32* %16, i64 1
  %19 = load i32, i32* %5, align 4
  store i32 %19, i32* %18, align 4
  %20 = getelementptr inbounds i32, i32* %18, i64 1
  %21 = load i32, i32* %6, align 4
  store i32 %21, i32* %20, align 4
  %22 = getelementptr inbounds [2 x [3 x i32]], [2 x [3 x i32]]* %7, i64 0, i64 1
  %23 = getelementptr inbounds [3 x i32], [3 x i32]* %22, i64 0, i64 0
  %24 = load i32, i32* %23, align 4
  ret i32 %24
}

attributes #0 = { noinline nounwind optnone uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.0"}
