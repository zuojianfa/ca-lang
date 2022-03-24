; ModuleID = 'landor.c'
source_filename = "landor.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @func1() #0 {
  ret i32 1
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  store i32 1, i32* %2, align 4
  store i32 2, i32* %3, align 4
  %10 = load i32, i32* %2, align 4
  %11 = load i32, i32* %3, align 4
  %12 = icmp slt i32 %10, %11
  %13 = zext i1 %12 to i32
  store i32 %13, i32* %4, align 4
  %14 = load i32, i32* %2, align 4
  %15 = load i32, i32* %3, align 4
  %16 = icmp sgt i32 %14, %15
  %17 = zext i1 %16 to i32
  store i32 %17, i32* %5, align 4
  %18 = load i32, i32* %4, align 4
  %19 = icmp ne i32 %18, 0
  br i1 %19, label %20, label %23

20:                                               ; preds = %0
  %21 = call i32 @func1()
  %22 = icmp ne i32 %21, 0
  br label %23

23:                                               ; preds = %20, %0
  %24 = phi i1 [ false, %0 ], [ %22, %20 ]
  %25 = zext i1 %24 to i32
  store i32 %25, i32* %6, align 4
  %26 = load i32, i32* %4, align 4
  %27 = icmp ne i32 %26, 0
  br i1 %27, label %31, label %28

28:                                               ; preds = %23
  %29 = call i32 @func1()
  %30 = icmp ne i32 %29, 0
  br label %31

31:                                               ; preds = %28, %23
  %32 = phi i1 [ true, %23 ], [ %30, %28 ]
  %33 = zext i1 %32 to i32
  store i32 %33, i32* %7, align 4
  %34 = load i32, i32* %5, align 4
  %35 = icmp ne i32 %34, 0
  br i1 %35, label %36, label %39

36:                                               ; preds = %31
  %37 = call i32 @func1()
  %38 = icmp ne i32 %37, 0
  br label %39

39:                                               ; preds = %36, %31
  %40 = phi i1 [ false, %31 ], [ %38, %36 ]
  %41 = zext i1 %40 to i32
  store i32 %41, i32* %8, align 4
  %42 = load i32, i32* %5, align 4
  %43 = icmp ne i32 %42, 0
  br i1 %43, label %47, label %44

44:                                               ; preds = %39
  %45 = call i32 @func1()
  %46 = icmp ne i32 %45, 0
  br label %47

47:                                               ; preds = %44, %39
  %48 = phi i1 [ true, %39 ], [ %46, %44 ]
  %49 = zext i1 %48 to i32
  store i32 %49, i32* %9, align 4
  ret i32 0
}

attributes #0 = { noinline nounwind optnone uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.0"}
