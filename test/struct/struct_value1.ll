; ModuleID = 'struct_value1.c'
source_filename = "struct_value1.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.AA = type { i32, double }

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca double, align 8
  %4 = alloca %struct.AA, align 8
  store i32 0, i32* %1, align 4
  store i32 305419896, i32* %2, align 4
  store double 0x400921FB4D12D84A, double* %3, align 8
  %5 = getelementptr inbounds %struct.AA, %struct.AA* %4, i32 0, i32 0
  %6 = load i32, i32* %2, align 4
  store i32 %6, i32* %5, align 8
  %7 = getelementptr inbounds %struct.AA, %struct.AA* %4, i32 0, i32 1
  %8 = load double, double* %3, align 8
  store double %8, double* %7, align 8
  ret i32 0
}

attributes #0 = { noinline nounwind optnone uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.0"}
