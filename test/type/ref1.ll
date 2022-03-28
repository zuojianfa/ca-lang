; ModuleID = 'ref1.cpp'
source_filename = "ref1.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.AA = type { i8*, double* }

; Function Attrs: noinline norecurse nounwind optnone uwtable mustprogress
define dso_local i32 @main() #0 {
  %1 = alloca i8, align 1
  %2 = alloca double, align 8
  %3 = alloca %struct.AA, align 8
  store i8 49, i8* %1, align 1
  store double 2.200000e+00, double* %2, align 8
  %4 = getelementptr inbounds %struct.AA, %struct.AA* %3, i32 0, i32 0
  store i8* %1, i8** %4, align 8
  %5 = getelementptr inbounds %struct.AA, %struct.AA* %3, i32 0, i32 1
  store double* %2, double** %5, align 8
  ret i32 0
}

attributes #0 = { noinline norecurse nounwind optnone uwtable mustprogress "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.0"}
