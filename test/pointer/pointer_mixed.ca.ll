; ModuleID = 'pointer_mixed.ca'
source_filename = "pointer_mixed.ca"

declare i32 @printf(i8*, ...)

define void @main() {
entry:
  %vi8 = alloca i8*, align 8
  store volatile i8* inttoptr (i32 1234 to i8*), i8** %vi8, align 8
  %vi32 = alloca i32*, align 8
  store volatile i32* inttoptr (i32 1234 to i32*), i32** %vi32, align 8
  %vi64 = alloca i64*, align 8
  store volatile i64* inttoptr (i32 1234 to i64*), i64** %vi64, align 8
  %vu8 = alloca i8*, align 8
  store volatile i8* inttoptr (i32 1234 to i8*), i8** %vu8, align 8
  %vu32 = alloca i32*, align 8
  store volatile i32* inttoptr (i32 1234 to i32*), i32** %vu32, align 8
  %vu64 = alloca i64*, align 8
  store volatile i64* inttoptr (i32 1234 to i64*), i64** %vu64, align 8
  %vf32 = alloca float*, align 8
  store volatile float* inttoptr (i32 1234 to float*), float** %vf32, align 8
  %vf64 = alloca double*, align 8
  store volatile double* inttoptr (i32 1234 to double*), double** %vf64, align 8
  %visize = alloca i64*, align 8
  store volatile i64* inttoptr (i32 1234 to i64*), i64** %visize, align 8
  %vusize = alloca i64*, align 8
  store volatile i64* inttoptr (i32 1234 to i64*), i64** %vusize, align 8
  %vvoid = alloca void*, align 8
  store volatile void* inttoptr (i32 1234 to void*), void** %vvoid, align 8
  %vchar = alloca i8*, align 8
  store volatile i8* inttoptr (i32 1234 to i8*), i8** %vchar, align 8
  %vuchar = alloca i8*, align 8
  store volatile i8* inttoptr (i32 1234 to i8*), i8** %vuchar, align 8
  %vbool = alloca i1*, align 8
  store volatile i1* inttoptr (i32 1234 to i1*), i1** %vbool, align 8
  %tmpexpr = load i8*, i8** %vi8, align 8
  %0 = bitcast i8* %tmpexpr to float**
  %pvi8 = alloca float**, align 8
  store volatile float** %0, float*** %pvi8, align 8
  %tmpexpr1 = load i32*, i32** %vi32, align 8
  %1 = bitcast i32* %tmpexpr1 to double**
  %pvi32 = alloca double**, align 8
  store volatile double** %1, double*** %pvi32, align 8
  %tmpexpr2 = load i64*, i64** %vi64, align 8
  %2 = bitcast i64* %tmpexpr2 to i64**
  %pvi64 = alloca i64**, align 8
  store volatile i64** %2, i64*** %pvi64, align 8
  %tmpexpr3 = load i8*, i8** %vu8, align 8
  %3 = bitcast i8* %tmpexpr3 to i64**
  %pvu8 = alloca i64**, align 8
  store volatile i64** %3, i64*** %pvu8, align 8
  %tmpexpr4 = load i32*, i32** %vu32, align 8
  %4 = bitcast i32* %tmpexpr4 to void**
  %pvu32 = alloca void**, align 8
  store volatile void** %4, void*** %pvu32, align 8
  %tmpexpr5 = load i64*, i64** %vu64, align 8
  %5 = bitcast i64* %tmpexpr5 to i8**
  %pvu64 = alloca i8**, align 8
  store volatile i8** %5, i8*** %pvu64, align 8
  %tmpexpr6 = load float*, float** %vf32, align 8
  %6 = bitcast float* %tmpexpr6 to i8**
  %pvf32 = alloca i8**, align 8
  store volatile i8** %6, i8*** %pvf32, align 8
  %tmpexpr7 = load double*, double** %vf64, align 8
  %7 = bitcast double* %tmpexpr7 to i1**
  %pvf64 = alloca i1**, align 8
  store volatile i1** %7, i1*** %pvf64, align 8
  %tmpexpr8 = load i64*, i64** %visize, align 8
  %8 = bitcast i64* %tmpexpr8 to i8**
  %pvisize = alloca i8**, align 8
  store volatile i8** %8, i8*** %pvisize, align 8
  %tmpexpr9 = load i64*, i64** %vusize, align 8
  %9 = bitcast i64* %tmpexpr9 to i32**
  %pvusize = alloca i32**, align 8
  store volatile i32** %9, i32*** %pvusize, align 8
  %tmpexpr10 = load void*, void** %vvoid, align 8
  %10 = bitcast void* %tmpexpr10 to i64**
  %pvvoid = alloca i64**, align 8
  store volatile i64** %10, i64*** %pvvoid, align 8
  %tmpexpr11 = load i8*, i8** %vchar, align 8
  %11 = bitcast i8* %tmpexpr11 to i8**
  %pvchar = alloca i8**, align 8
  store volatile i8** %11, i8*** %pvchar, align 8
  %tmpexpr12 = load i8*, i8** %vuchar, align 8
  %12 = bitcast i8* %tmpexpr12 to i32**
  %pvuchar = alloca i32**, align 8
  store volatile i32** %12, i32*** %pvuchar, align 8
  %tmpexpr13 = load i1*, i1** %vbool, align 8
  %13 = bitcast i1* %tmpexpr13 to i64**
  %pvbool = alloca i64**, align 8
  store volatile i64** %13, i64*** %pvbool, align 8
  %tmpexpr14 = load i8*, i8** %vi8, align 8
  %14 = bitcast i8* %tmpexpr14 to float**
  %p2vi8 = alloca float**, align 8
  store volatile float** %14, float*** %p2vi8, align 8
  %tmpexpr15 = load i32*, i32** %vi32, align 8
  %15 = bitcast i32* %tmpexpr15 to double**
  %p2vi32 = alloca double**, align 8
  store volatile double** %15, double*** %p2vi32, align 8
  %tmpexpr16 = load i64*, i64** %vi64, align 8
  %16 = bitcast i64* %tmpexpr16 to i64**
  %p2vi64 = alloca i64**, align 8
  store volatile i64** %16, i64*** %p2vi64, align 8
  %tmpexpr17 = load i8*, i8** %vu8, align 8
  %17 = bitcast i8* %tmpexpr17 to i64**
  %p2vu8 = alloca i64**, align 8
  store volatile i64** %17, i64*** %p2vu8, align 8
  %tmpexpr18 = load i32*, i32** %vu32, align 8
  %18 = bitcast i32* %tmpexpr18 to void**
  %p2vu32 = alloca void**, align 8
  store volatile void** %18, void*** %p2vu32, align 8
  %tmpexpr19 = load i64*, i64** %vu64, align 8
  %19 = bitcast i64* %tmpexpr19 to i8**
  %p2vu64 = alloca i8**, align 8
  store volatile i8** %19, i8*** %p2vu64, align 8
  %tmpexpr20 = load float*, float** %vf32, align 8
  %20 = bitcast float* %tmpexpr20 to i8**
  %p2vf32 = alloca i8**, align 8
  store volatile i8** %20, i8*** %p2vf32, align 8
  %tmpexpr21 = load double*, double** %vf64, align 8
  %21 = bitcast double* %tmpexpr21 to i1**
  %p2vf64 = alloca i1**, align 8
  store volatile i1** %21, i1*** %p2vf64, align 8
  %tmpexpr22 = load i64*, i64** %visize, align 8
  %22 = bitcast i64* %tmpexpr22 to i8**
  %p2visize = alloca i8**, align 8
  store volatile i8** %22, i8*** %p2visize, align 8
  %tmpexpr23 = load i64*, i64** %vusize, align 8
  %23 = bitcast i64* %tmpexpr23 to i32**
  %p2vusize = alloca i32**, align 8
  store volatile i32** %23, i32*** %p2vusize, align 8
  %tmpexpr24 = load void*, void** %vvoid, align 8
  %24 = bitcast void* %tmpexpr24 to i64**
  %p2vvoid = alloca i64**, align 8
  store volatile i64** %24, i64*** %p2vvoid, align 8
  %tmpexpr25 = load i8*, i8** %vchar, align 8
  %25 = bitcast i8* %tmpexpr25 to i8**
  %p2vchar = alloca i8**, align 8
  store volatile i8** %25, i8*** %p2vchar, align 8
  %tmpexpr26 = load i8*, i8** %vuchar, align 8
  %26 = bitcast i8* %tmpexpr26 to i32**
  %p2vuchar = alloca i32**, align 8
  store volatile i32** %26, i32*** %p2vuchar, align 8
  %tmpexpr27 = load i1*, i1** %vbool, align 8
  %27 = bitcast i1* %tmpexpr27 to i64**
  %p2vbool = alloca i64**, align 8
  store volatile i64** %27, i64*** %p2vbool, align 8
  %tmpexpr28 = load i8*, i8** %vi8, align 8
  %28 = bitcast i8* %tmpexpr28 to float***
  %ppvi8 = alloca float***, align 8
  store volatile float*** %28, float**** %ppvi8, align 8
  %tmpexpr29 = load i32*, i32** %vi32, align 8
  %29 = bitcast i32* %tmpexpr29 to double***
  %ppvi32 = alloca double***, align 8
  store volatile double*** %29, double**** %ppvi32, align 8
  %tmpexpr30 = load i64*, i64** %vi64, align 8
  %30 = bitcast i64* %tmpexpr30 to i64***
  %ppvi64 = alloca i64***, align 8
  store volatile i64*** %30, i64**** %ppvi64, align 8
  %tmpexpr31 = load i8*, i8** %vu8, align 8
  %31 = bitcast i8* %tmpexpr31 to i64***
  %ppvu8 = alloca i64***, align 8
  store volatile i64*** %31, i64**** %ppvu8, align 8
  %tmpexpr32 = load i32*, i32** %vu32, align 8
  %32 = bitcast i32* %tmpexpr32 to void***
  %ppvu32 = alloca void***, align 8
  store volatile void*** %32, void**** %ppvu32, align 8
  %tmpexpr33 = load i64*, i64** %vu64, align 8
  %33 = bitcast i64* %tmpexpr33 to i8***
  %ppvu64 = alloca i8***, align 8
  store volatile i8*** %33, i8**** %ppvu64, align 8
  %tmpexpr34 = load float*, float** %vf32, align 8
  %34 = bitcast float* %tmpexpr34 to i8***
  %ppvf32 = alloca i8***, align 8
  store volatile i8*** %34, i8**** %ppvf32, align 8
  %tmpexpr35 = load double*, double** %vf64, align 8
  %35 = bitcast double* %tmpexpr35 to i1***
  %ppvf64 = alloca i1***, align 8
  store volatile i1*** %35, i1**** %ppvf64, align 8
  %tmpexpr36 = load i64*, i64** %visize, align 8
  %36 = bitcast i64* %tmpexpr36 to i8***
  %ppvisize = alloca i8***, align 8
  store volatile i8*** %36, i8**** %ppvisize, align 8
  %tmpexpr37 = load i64*, i64** %vusize, align 8
  %37 = bitcast i64* %tmpexpr37 to i32***
  %ppvusize = alloca i32***, align 8
  store volatile i32*** %37, i32**** %ppvusize, align 8
  %tmpexpr38 = load void*, void** %vvoid, align 8
  %38 = bitcast void* %tmpexpr38 to i64***
  %ppvvoid = alloca i64***, align 8
  store volatile i64*** %38, i64**** %ppvvoid, align 8
  %tmpexpr39 = load i8*, i8** %vchar, align 8
  %39 = bitcast i8* %tmpexpr39 to i8***
  %ppvchar = alloca i8***, align 8
  store volatile i8*** %39, i8**** %ppvchar, align 8
  %tmpexpr40 = load i8*, i8** %vuchar, align 8
  %40 = bitcast i8* %tmpexpr40 to i32***
  %ppvuchar = alloca i32***, align 8
  store volatile i32*** %40, i32**** %ppvuchar, align 8
  %tmpexpr41 = load i1*, i1** %vbool, align 8
  %41 = bitcast i1* %tmpexpr41 to i64***
  %ppvbool = alloca i64***, align 8
  store volatile i64*** %41, i64**** %ppvbool, align 8
  %tmpexpr42 = load i8*, i8** %vi8, align 8
  %42 = bitcast i8* %tmpexpr42 to float***
  %pp2vi8 = alloca float***, align 8
  store volatile float*** %42, float**** %pp2vi8, align 8
  %tmpexpr43 = load i32*, i32** %vi32, align 8
  %43 = bitcast i32* %tmpexpr43 to double***
  %pp2vi32 = alloca double***, align 8
  store volatile double*** %43, double**** %pp2vi32, align 8
  %tmpexpr44 = load i64*, i64** %vi64, align 8
  %44 = bitcast i64* %tmpexpr44 to i64***
  %pp2vi64 = alloca i64***, align 8
  store volatile i64*** %44, i64**** %pp2vi64, align 8
  %tmpexpr45 = load i8*, i8** %vu8, align 8
  %45 = bitcast i8* %tmpexpr45 to i64***
  %pp2vu8 = alloca i64***, align 8
  store volatile i64*** %45, i64**** %pp2vu8, align 8
  %tmpexpr46 = load i32*, i32** %vu32, align 8
  %46 = bitcast i32* %tmpexpr46 to void***
  %pp2vu32 = alloca void***, align 8
  store volatile void*** %46, void**** %pp2vu32, align 8
  %tmpexpr47 = load i64*, i64** %vu64, align 8
  %47 = bitcast i64* %tmpexpr47 to i8***
  %pp2vu64 = alloca i8***, align 8
  store volatile i8*** %47, i8**** %pp2vu64, align 8
  %tmpexpr48 = load float*, float** %vf32, align 8
  %48 = bitcast float* %tmpexpr48 to i8***
  %pp2vf32 = alloca i8***, align 8
  store volatile i8*** %48, i8**** %pp2vf32, align 8
  %tmpexpr49 = load double*, double** %vf64, align 8
  %49 = bitcast double* %tmpexpr49 to i1***
  %pp2vf64 = alloca i1***, align 8
  store volatile i1*** %49, i1**** %pp2vf64, align 8
  %tmpexpr50 = load i64*, i64** %visize, align 8
  %50 = bitcast i64* %tmpexpr50 to i8***
  %pp2visize = alloca i8***, align 8
  store volatile i8*** %50, i8**** %pp2visize, align 8
  %tmpexpr51 = load i64*, i64** %vusize, align 8
  %51 = bitcast i64* %tmpexpr51 to i32***
  %pp2vusize = alloca i32***, align 8
  store volatile i32*** %51, i32**** %pp2vusize, align 8
  %tmpexpr52 = load void*, void** %vvoid, align 8
  %52 = bitcast void* %tmpexpr52 to i64***
  %pp2vvoid = alloca i64***, align 8
  store volatile i64*** %52, i64**** %pp2vvoid, align 8
  %tmpexpr53 = load i8*, i8** %vchar, align 8
  %53 = bitcast i8* %tmpexpr53 to i8***
  %pp2vchar = alloca i8***, align 8
  store volatile i8*** %53, i8**** %pp2vchar, align 8
  %tmpexpr54 = load i8*, i8** %vuchar, align 8
  %54 = bitcast i8* %tmpexpr54 to i32***
  %pp2vuchar = alloca i32***, align 8
  store volatile i32*** %54, i32**** %pp2vuchar, align 8
  %tmpexpr55 = load i1*, i1** %vbool, align 8
  %55 = bitcast i1* %tmpexpr55 to i64***
  %pp2vbool = alloca i64***, align 8
  store volatile i64*** %55, i64**** %pp2vbool, align 8
  %tmpexpr56 = load i8*, i8** %vi8, align 8
  %56 = bitcast i8* %tmpexpr56 to float****
  %pppvi8 = alloca float****, align 8
  store volatile float**** %56, float***** %pppvi8, align 8
  %tmpexpr57 = load i32*, i32** %vi32, align 8
  %57 = bitcast i32* %tmpexpr57 to double****
  %pppvi32 = alloca double****, align 8
  store volatile double**** %57, double***** %pppvi32, align 8
  %tmpexpr58 = load i64*, i64** %vi64, align 8
  %58 = bitcast i64* %tmpexpr58 to i64****
  %pppvi64 = alloca i64****, align 8
  store volatile i64**** %58, i64***** %pppvi64, align 8
  %tmpexpr59 = load i8*, i8** %vu8, align 8
  %59 = bitcast i8* %tmpexpr59 to i64****
  %pppvu8 = alloca i64****, align 8
  store volatile i64**** %59, i64***** %pppvu8, align 8
  %tmpexpr60 = load i32*, i32** %vu32, align 8
  %60 = bitcast i32* %tmpexpr60 to void****
  %pppvu32 = alloca void****, align 8
  store volatile void**** %60, void***** %pppvu32, align 8
  %tmpexpr61 = load i64*, i64** %vu64, align 8
  %61 = bitcast i64* %tmpexpr61 to i8****
  %pppvu64 = alloca i8****, align 8
  store volatile i8**** %61, i8***** %pppvu64, align 8
  %tmpexpr62 = load float*, float** %vf32, align 8
  %62 = bitcast float* %tmpexpr62 to i8****
  %pppvf32 = alloca i8****, align 8
  store volatile i8**** %62, i8***** %pppvf32, align 8
  %tmpexpr63 = load double*, double** %vf64, align 8
  %63 = bitcast double* %tmpexpr63 to i1****
  %pppvf64 = alloca i1****, align 8
  store volatile i1**** %63, i1***** %pppvf64, align 8
  %tmpexpr64 = load i64*, i64** %visize, align 8
  %64 = bitcast i64* %tmpexpr64 to i8****
  %pppvisize = alloca i8****, align 8
  store volatile i8**** %64, i8***** %pppvisize, align 8
  %tmpexpr65 = load i64*, i64** %vusize, align 8
  %65 = bitcast i64* %tmpexpr65 to i32****
  %pppvusize = alloca i32****, align 8
  store volatile i32**** %65, i32***** %pppvusize, align 8
  %tmpexpr66 = load void*, void** %vvoid, align 8
  %66 = bitcast void* %tmpexpr66 to i64****
  %pppvvoid = alloca i64****, align 8
  store volatile i64**** %66, i64***** %pppvvoid, align 8
  %tmpexpr67 = load i8*, i8** %vchar, align 8
  %67 = bitcast i8* %tmpexpr67 to i8****
  %pppvchar = alloca i8****, align 8
  store volatile i8**** %67, i8***** %pppvchar, align 8
  %tmpexpr68 = load i8*, i8** %vuchar, align 8
  %68 = bitcast i8* %tmpexpr68 to i32****
  %pppvuchar = alloca i32****, align 8
  store volatile i32**** %68, i32***** %pppvuchar, align 8
  %tmpexpr69 = load i1*, i1** %vbool, align 8
  %69 = bitcast i1* %tmpexpr69 to i64****
  %pppvbool = alloca i64****, align 8
  store volatile i64**** %69, i64***** %pppvbool, align 8
  %tmpexpr70 = load i8*, i8** %vi8, align 8
  %70 = bitcast i8* %tmpexpr70 to float****
  %ppp2vi8 = alloca float****, align 8
  store volatile float**** %70, float***** %ppp2vi8, align 8
  %tmpexpr71 = load i32*, i32** %vi32, align 8
  %71 = bitcast i32* %tmpexpr71 to double****
  %ppp2vi32 = alloca double****, align 8
  store volatile double**** %71, double***** %ppp2vi32, align 8
  %tmpexpr72 = load i64*, i64** %vi64, align 8
  %72 = bitcast i64* %tmpexpr72 to i64****
  %ppp2vi64 = alloca i64****, align 8
  store volatile i64**** %72, i64***** %ppp2vi64, align 8
  %tmpexpr73 = load i8*, i8** %vu8, align 8
  %73 = bitcast i8* %tmpexpr73 to i64****
  %ppp2vu8 = alloca i64****, align 8
  store volatile i64**** %73, i64***** %ppp2vu8, align 8
  %tmpexpr74 = load i32*, i32** %vu32, align 8
  %74 = bitcast i32* %tmpexpr74 to void****
  %ppp2vu32 = alloca void****, align 8
  store volatile void**** %74, void***** %ppp2vu32, align 8
  %tmpexpr75 = load i64*, i64** %vu64, align 8
  %75 = bitcast i64* %tmpexpr75 to i8****
  %ppp2vu64 = alloca i8****, align 8
  store volatile i8**** %75, i8***** %ppp2vu64, align 8
  %tmpexpr76 = load float*, float** %vf32, align 8
  %76 = bitcast float* %tmpexpr76 to i8****
  %ppp2vf32 = alloca i8****, align 8
  store volatile i8**** %76, i8***** %ppp2vf32, align 8
  %tmpexpr77 = load double*, double** %vf64, align 8
  %77 = bitcast double* %tmpexpr77 to i1****
  %ppp2vf64 = alloca i1****, align 8
  store volatile i1**** %77, i1***** %ppp2vf64, align 8
  %tmpexpr78 = load i64*, i64** %visize, align 8
  %78 = bitcast i64* %tmpexpr78 to i8****
  %ppp2visize = alloca i8****, align 8
  store volatile i8**** %78, i8***** %ppp2visize, align 8
  %tmpexpr79 = load i64*, i64** %vusize, align 8
  %79 = bitcast i64* %tmpexpr79 to i32****
  %ppp2vusize = alloca i32****, align 8
  store volatile i32**** %79, i32***** %ppp2vusize, align 8
  %tmpexpr80 = load void*, void** %vvoid, align 8
  %80 = bitcast void* %tmpexpr80 to i64****
  %ppp2vvoid = alloca i64****, align 8
  store volatile i64**** %80, i64***** %ppp2vvoid, align 8
  %tmpexpr81 = load i8*, i8** %vchar, align 8
  %81 = bitcast i8* %tmpexpr81 to i8****
  %ppp2vchar = alloca i8****, align 8
  store volatile i8**** %81, i8***** %ppp2vchar, align 8
  %tmpexpr82 = load i8*, i8** %vuchar, align 8
  %82 = bitcast i8* %tmpexpr82 to i32****
  %ppp2vuchar = alloca i32****, align 8
  store volatile i32**** %82, i32***** %ppp2vuchar, align 8
  %tmpexpr83 = load i1*, i1** %vbool, align 8
  %83 = bitcast i1* %tmpexpr83 to i64****
  %ppp2vbool = alloca i64****, align 8
  store volatile i64**** %83, i64***** %ppp2vbool, align 8
  %tmpexpr84 = load float****, float***** %ppp2vi8, align 8
  %84 = bitcast float**** %tmpexpr84 to i8*
  %svi8 = alloca i8*, align 8
  store volatile i8* %84, i8** %svi8, align 8
  %tmpexpr85 = load double****, double***** %ppp2vi32, align 8
  %85 = bitcast double**** %tmpexpr85 to i32*
  %svi32 = alloca i32*, align 8
  store volatile i32* %85, i32** %svi32, align 8
  %tmpexpr86 = load i64****, i64***** %ppp2vi64, align 8
  %86 = bitcast i64**** %tmpexpr86 to i64*
  %svi64 = alloca i64*, align 8
  store volatile i64* %86, i64** %svi64, align 8
  %tmpexpr87 = load i64****, i64***** %ppp2vu8, align 8
  %87 = bitcast i64**** %tmpexpr87 to i8*
  %svu8 = alloca i8*, align 8
  store volatile i8* %87, i8** %svu8, align 8
  %tmpexpr88 = load void****, void***** %ppp2vu32, align 8
  %88 = bitcast void**** %tmpexpr88 to i32*
  %svu32 = alloca i32*, align 8
  store volatile i32* %88, i32** %svu32, align 8
  %tmpexpr89 = load i8****, i8***** %ppp2vu64, align 8
  %89 = bitcast i8**** %tmpexpr89 to i64*
  %svu64 = alloca i64*, align 8
  store volatile i64* %89, i64** %svu64, align 8
  %tmpexpr90 = load i8****, i8***** %ppp2vf32, align 8
  %90 = bitcast i8**** %tmpexpr90 to float*
  %svf32 = alloca float*, align 8
  store volatile float* %90, float** %svf32, align 8
  %tmpexpr91 = load i1****, i1***** %ppp2vf64, align 8
  %91 = bitcast i1**** %tmpexpr91 to double*
  %svf64 = alloca double*, align 8
  store volatile double* %91, double** %svf64, align 8
  %tmpexpr92 = load i8****, i8***** %ppp2visize, align 8
  %92 = bitcast i8**** %tmpexpr92 to i64*
  %svisize = alloca i64*, align 8
  store volatile i64* %92, i64** %svisize, align 8
  %tmpexpr93 = load i32****, i32***** %ppp2vusize, align 8
  %93 = bitcast i32**** %tmpexpr93 to i64*
  %svusize = alloca i64*, align 8
  store volatile i64* %93, i64** %svusize, align 8
  %tmpexpr94 = load i64****, i64***** %ppp2vvoid, align 8
  %94 = bitcast i64**** %tmpexpr94 to void*
  %svvoid = alloca void*, align 8
  store volatile void* %94, void** %svvoid, align 8
  %tmpexpr95 = load i8****, i8***** %ppp2vchar, align 8
  %95 = bitcast i8**** %tmpexpr95 to i8*
  %svchar = alloca i8*, align 8
  store volatile i8* %95, i8** %svchar, align 8
  %tmpexpr96 = load i32****, i32***** %ppp2vuchar, align 8
  %96 = bitcast i32**** %tmpexpr96 to i8*
  %svuchar = alloca i8*, align 8
  store volatile i8* %96, i8** %svuchar, align 8
  %tmpexpr97 = load i64****, i64***** %ppp2vbool, align 8
  %97 = bitcast i64**** %tmpexpr97 to i1*
  %svbool = alloca i1*, align 8
  store volatile i1* %97, i1** %svbool, align 8
  br label %ret

ret:                                              ; preds = %entry
  ret void
}
