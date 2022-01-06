; ModuleID = 'pointer2_notzero.ca'
source_filename = "pointer2_notzero.ca"

declare i32 @printf(i8*, ...)

define void @main() {
entry:
  %vi8 = alloca i8**, align 8
  store volatile i8** inttoptr (i64 4324324132432 to i8**), i8*** %vi8, align 8
  %vi32 = alloca i32**, align 8
  store volatile i32** inttoptr (i64 4324324132432 to i32**), i32*** %vi32, align 8
  %vi64 = alloca i64**, align 8
  store volatile i64** inttoptr (i64 4324324132432 to i64**), i64*** %vi64, align 8
  %vu8 = alloca i8**, align 8
  store volatile i8** inttoptr (i64 4324324132432 to i8**), i8*** %vu8, align 8
  %vu32 = alloca i32**, align 8
  store volatile i32** inttoptr (i64 4324324132432 to i32**), i32*** %vu32, align 8
  %vu64 = alloca i64**, align 8
  store volatile i64** inttoptr (i64 4324324132432 to i64**), i64*** %vu64, align 8
  %vf32 = alloca float**, align 8
  store volatile float** inttoptr (i64 4324324132432 to float**), float*** %vf32, align 8
  %vf64 = alloca double**, align 8
  store volatile double** inttoptr (i64 4324324132432 to double**), double*** %vf64, align 8
  %visize = alloca i64**, align 8
  store volatile i64** inttoptr (i64 4324324132432 to i64**), i64*** %visize, align 8
  %vusize = alloca i64**, align 8
  store volatile i64** inttoptr (i64 4324324132432 to i64**), i64*** %vusize, align 8
  %vvoid = alloca void**, align 8
  store volatile void** inttoptr (i64 4324324132432 to void**), void*** %vvoid, align 8
  %vchar = alloca i8**, align 8
  store volatile i8** inttoptr (i64 4324324132432 to i8**), i8*** %vchar, align 8
  %vuchar = alloca i8**, align 8
  store volatile i8** inttoptr (i64 4324324132432 to i8**), i8*** %vuchar, align 8
  %vbool = alloca i1**, align 8
  store volatile i1** inttoptr (i64 4324324132432 to i1**), i1*** %vbool, align 8
  %tmpexpr = load i8**, i8*** %vi8, align 8
  %0 = bitcast i8** %tmpexpr to i8***
  %pvi8 = alloca i8***, align 8
  store volatile i8*** %0, i8**** %pvi8, align 8
  %tmpexpr1 = load i32**, i32*** %vi32, align 8
  %1 = bitcast i32** %tmpexpr1 to i32***
  %pvi32 = alloca i32***, align 8
  store volatile i32*** %1, i32**** %pvi32, align 8
  %tmpexpr2 = load i64**, i64*** %vi64, align 8
  %2 = bitcast i64** %tmpexpr2 to i64***
  %pvi64 = alloca i64***, align 8
  store volatile i64*** %2, i64**** %pvi64, align 8
  %tmpexpr3 = load i8**, i8*** %vu8, align 8
  %3 = bitcast i8** %tmpexpr3 to i8***
  %pvu8 = alloca i8***, align 8
  store volatile i8*** %3, i8**** %pvu8, align 8
  %tmpexpr4 = load i32**, i32*** %vu32, align 8
  %4 = bitcast i32** %tmpexpr4 to i32***
  %pvu32 = alloca i32***, align 8
  store volatile i32*** %4, i32**** %pvu32, align 8
  %tmpexpr5 = load i64**, i64*** %vu64, align 8
  %5 = bitcast i64** %tmpexpr5 to i64***
  %pvu64 = alloca i64***, align 8
  store volatile i64*** %5, i64**** %pvu64, align 8
  %tmpexpr6 = load float**, float*** %vf32, align 8
  %6 = bitcast float** %tmpexpr6 to float***
  %pvf32 = alloca float***, align 8
  store volatile float*** %6, float**** %pvf32, align 8
  %tmpexpr7 = load double**, double*** %vf64, align 8
  %7 = bitcast double** %tmpexpr7 to double***
  %pvf64 = alloca double***, align 8
  store volatile double*** %7, double**** %pvf64, align 8
  %tmpexpr8 = load i64**, i64*** %visize, align 8
  %8 = bitcast i64** %tmpexpr8 to i64***
  %pvisize = alloca i64***, align 8
  store volatile i64*** %8, i64**** %pvisize, align 8
  %tmpexpr9 = load i64**, i64*** %vusize, align 8
  %9 = bitcast i64** %tmpexpr9 to i64***
  %pvusize = alloca i64***, align 8
  store volatile i64*** %9, i64**** %pvusize, align 8
  %tmpexpr10 = load void**, void*** %vvoid, align 8
  %10 = bitcast void** %tmpexpr10 to void***
  %pvvoid = alloca void***, align 8
  store volatile void*** %10, void**** %pvvoid, align 8
  %tmpexpr11 = load i8**, i8*** %vchar, align 8
  %11 = bitcast i8** %tmpexpr11 to i8***
  %pvchar = alloca i8***, align 8
  store volatile i8*** %11, i8**** %pvchar, align 8
  %tmpexpr12 = load i8**, i8*** %vuchar, align 8
  %12 = bitcast i8** %tmpexpr12 to i8***
  %pvuchar = alloca i8***, align 8
  store volatile i8*** %12, i8**** %pvuchar, align 8
  %tmpexpr13 = load i1**, i1*** %vbool, align 8
  %13 = bitcast i1** %tmpexpr13 to i1***
  %pvbool = alloca i1***, align 8
  store volatile i1*** %13, i1**** %pvbool, align 8
  %tmpexpr14 = load i8**, i8*** %vi8, align 8
  %14 = bitcast i8** %tmpexpr14 to i8***
  %p2vi8 = alloca i8***, align 8
  store volatile i8*** %14, i8**** %p2vi8, align 8
  %tmpexpr15 = load i32**, i32*** %vi32, align 8
  %15 = bitcast i32** %tmpexpr15 to i32***
  %p2vi32 = alloca i32***, align 8
  store volatile i32*** %15, i32**** %p2vi32, align 8
  %tmpexpr16 = load i64**, i64*** %vi64, align 8
  %16 = bitcast i64** %tmpexpr16 to i64***
  %p2vi64 = alloca i64***, align 8
  store volatile i64*** %16, i64**** %p2vi64, align 8
  %tmpexpr17 = load i8**, i8*** %vu8, align 8
  %17 = bitcast i8** %tmpexpr17 to i8***
  %p2vu8 = alloca i8***, align 8
  store volatile i8*** %17, i8**** %p2vu8, align 8
  %tmpexpr18 = load i32**, i32*** %vu32, align 8
  %18 = bitcast i32** %tmpexpr18 to i32***
  %p2vu32 = alloca i32***, align 8
  store volatile i32*** %18, i32**** %p2vu32, align 8
  %tmpexpr19 = load i64**, i64*** %vu64, align 8
  %19 = bitcast i64** %tmpexpr19 to i64***
  %p2vu64 = alloca i64***, align 8
  store volatile i64*** %19, i64**** %p2vu64, align 8
  %tmpexpr20 = load float**, float*** %vf32, align 8
  %20 = bitcast float** %tmpexpr20 to float***
  %p2vf32 = alloca float***, align 8
  store volatile float*** %20, float**** %p2vf32, align 8
  %tmpexpr21 = load double**, double*** %vf64, align 8
  %21 = bitcast double** %tmpexpr21 to double***
  %p2vf64 = alloca double***, align 8
  store volatile double*** %21, double**** %p2vf64, align 8
  %tmpexpr22 = load i64**, i64*** %visize, align 8
  %22 = bitcast i64** %tmpexpr22 to i64***
  %p2visize = alloca i64***, align 8
  store volatile i64*** %22, i64**** %p2visize, align 8
  %tmpexpr23 = load i64**, i64*** %vusize, align 8
  %23 = bitcast i64** %tmpexpr23 to i64***
  %p2vusize = alloca i64***, align 8
  store volatile i64*** %23, i64**** %p2vusize, align 8
  %tmpexpr24 = load void**, void*** %vvoid, align 8
  %24 = bitcast void** %tmpexpr24 to void***
  %p2vvoid = alloca void***, align 8
  store volatile void*** %24, void**** %p2vvoid, align 8
  %tmpexpr25 = load i8**, i8*** %vchar, align 8
  %25 = bitcast i8** %tmpexpr25 to i8***
  %p2vchar = alloca i8***, align 8
  store volatile i8*** %25, i8**** %p2vchar, align 8
  %tmpexpr26 = load i8**, i8*** %vuchar, align 8
  %26 = bitcast i8** %tmpexpr26 to i8***
  %p2vuchar = alloca i8***, align 8
  store volatile i8*** %26, i8**** %p2vuchar, align 8
  %tmpexpr27 = load i1**, i1*** %vbool, align 8
  %27 = bitcast i1** %tmpexpr27 to i1***
  %p2vbool = alloca i1***, align 8
  store volatile i1*** %27, i1**** %p2vbool, align 8
  br label %ret

ret:                                              ; preds = %entry
  ret void
}
