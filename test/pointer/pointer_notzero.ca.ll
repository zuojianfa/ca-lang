; ModuleID = 'pointer_notzero.ca'
source_filename = "pointer_notzero.ca"

declare i32 @printf(i8*, ...)

define void @main() {
entry:
  %svbool = alloca i1*, align 8
  %svuchar = alloca i8*, align 8
  %svchar = alloca i8*, align 8
  %svvoid = alloca void*, align 8
  %svusize = alloca i64*, align 8
  %svisize = alloca i64*, align 8
  %svf64 = alloca double*, align 8
  %svf32 = alloca float*, align 8
  %svu64 = alloca i64*, align 8
  %svu32 = alloca i32*, align 8
  %svu8 = alloca i8*, align 8
  %svi64 = alloca i64*, align 8
  %svi32 = alloca i32*, align 8
  %svi8 = alloca i8*, align 8
  %ppp2vbool = alloca i1****, align 8
  %ppp2vuchar = alloca i8****, align 8
  %ppp2vchar = alloca i8****, align 8
  %ppp2vvoid = alloca void****, align 8
  %ppp2vusize = alloca i64****, align 8
  %ppp2visize = alloca i64****, align 8
  %ppp2vf64 = alloca double****, align 8
  %ppp2vf32 = alloca float****, align 8
  %ppp2vu64 = alloca i64****, align 8
  %ppp2vu32 = alloca i32****, align 8
  %ppp2vu8 = alloca i8****, align 8
  %ppp2vi64 = alloca i64****, align 8
  %ppp2vi32 = alloca i32****, align 8
  %ppp2vi8 = alloca i8****, align 8
  %pppvbool = alloca i1****, align 8
  %pppvuchar = alloca i8****, align 8
  %pppvchar = alloca i8****, align 8
  %pppvvoid = alloca void****, align 8
  %pppvusize = alloca i64****, align 8
  %pppvisize = alloca i64****, align 8
  %pppvf64 = alloca double****, align 8
  %pppvf32 = alloca float****, align 8
  %pppvu64 = alloca i64****, align 8
  %pppvu32 = alloca i32****, align 8
  %pppvu8 = alloca i8****, align 8
  %pppvi64 = alloca i64****, align 8
  %pppvi32 = alloca i32****, align 8
  %pppvi8 = alloca i8****, align 8
  %pp2vbool = alloca i1***, align 8
  %pp2vuchar = alloca i8***, align 8
  %pp2vchar = alloca i8***, align 8
  %pp2vvoid = alloca void***, align 8
  %pp2vusize = alloca i64***, align 8
  %pp2visize = alloca i64***, align 8
  %pp2vf64 = alloca double***, align 8
  %pp2vf32 = alloca float***, align 8
  %pp2vu64 = alloca i64***, align 8
  %pp2vu32 = alloca i32***, align 8
  %pp2vu8 = alloca i8***, align 8
  %pp2vi64 = alloca i64***, align 8
  %pp2vi32 = alloca i32***, align 8
  %pp2vi8 = alloca i8***, align 8
  %ppvbool = alloca i1***, align 8
  %ppvuchar = alloca i8***, align 8
  %ppvchar = alloca i8***, align 8
  %ppvvoid = alloca void***, align 8
  %ppvusize = alloca i64***, align 8
  %ppvisize = alloca i64***, align 8
  %ppvf64 = alloca double***, align 8
  %ppvf32 = alloca float***, align 8
  %ppvu64 = alloca i64***, align 8
  %ppvu32 = alloca i32***, align 8
  %ppvu8 = alloca i8***, align 8
  %ppvi64 = alloca i64***, align 8
  %ppvi32 = alloca i32***, align 8
  %ppvi8 = alloca i8***, align 8
  %p2vbool = alloca i1**, align 8
  %p2vuchar = alloca i8**, align 8
  %p2vchar = alloca i8**, align 8
  %p2vvoid = alloca void**, align 8
  %p2vusize = alloca i64**, align 8
  %p2visize = alloca i64**, align 8
  %p2vf64 = alloca double**, align 8
  %p2vf32 = alloca float**, align 8
  %p2vu64 = alloca i64**, align 8
  %p2vu32 = alloca i32**, align 8
  %p2vu8 = alloca i8**, align 8
  %p2vi64 = alloca i64**, align 8
  %p2vi32 = alloca i32**, align 8
  %p2vi8 = alloca i8**, align 8
  %pvbool = alloca i1**, align 8
  %pvuchar = alloca i8**, align 8
  %pvchar = alloca i8**, align 8
  %pvvoid = alloca void**, align 8
  %pvusize = alloca i64**, align 8
  %pvisize = alloca i64**, align 8
  %pvf64 = alloca double**, align 8
  %pvf32 = alloca float**, align 8
  %pvu64 = alloca i64**, align 8
  %pvu32 = alloca i32**, align 8
  %pvu8 = alloca i8**, align 8
  %pvi64 = alloca i64**, align 8
  %pvi32 = alloca i32**, align 8
  %pvi8 = alloca i8**, align 8
  %vbool = alloca i1*, align 8
  %vuchar = alloca i8*, align 8
  %vchar = alloca i8*, align 8
  %vvoid = alloca void*, align 8
  %vusize = alloca i64*, align 8
  %visize = alloca i64*, align 8
  %vf64 = alloca double*, align 8
  %vf32 = alloca float*, align 8
  %vu64 = alloca i64*, align 8
  %vu32 = alloca i32*, align 8
  %vu8 = alloca i8*, align 8
  %vi64 = alloca i64*, align 8
  %vi32 = alloca i32*, align 8
  %vi8 = alloca i8*, align 8
  store volatile i8* inttoptr (i64 4324343243 to i8*), i8** %vi8, align 8
  store volatile i32* inttoptr (i64 4324343243 to i32*), i32** %vi32, align 8
  store volatile i64* inttoptr (i64 4324343243 to i64*), i64** %vi64, align 8
  store volatile i8* inttoptr (i64 4324343243 to i8*), i8** %vu8, align 8
  store volatile i32* inttoptr (i64 4324343243 to i32*), i32** %vu32, align 8
  store volatile i64* inttoptr (i64 4324343243 to i64*), i64** %vu64, align 8
  store volatile float* inttoptr (i64 4324343243 to float*), float** %vf32, align 8
  store volatile double* inttoptr (i64 4324343243 to double*), double** %vf64, align 8
  store volatile i64* inttoptr (i64 4324343243 to i64*), i64** %visize, align 8
  store volatile i64* inttoptr (i64 4324343243 to i64*), i64** %vusize, align 8
  store volatile void* inttoptr (i64 4324343243 to void*), void** %vvoid, align 8
  store volatile i8* inttoptr (i64 4324343243 to i8*), i8** %vchar, align 8
  store volatile i8* inttoptr (i64 4324343243 to i8*), i8** %vuchar, align 8
  store volatile i1* inttoptr (i64 4324343243 to i1*), i1** %vbool, align 8
  %tmpexpr = load i8*, i8** %vi8, align 8
  %0 = bitcast i8* %tmpexpr to i8**
  store volatile i8** %0, i8*** %pvi8, align 8
  %tmpexpr1 = load i32*, i32** %vi32, align 8
  %1 = bitcast i32* %tmpexpr1 to i32**
  store volatile i32** %1, i32*** %pvi32, align 8
  %tmpexpr2 = load i64*, i64** %vi64, align 8
  %2 = bitcast i64* %tmpexpr2 to i64**
  store volatile i64** %2, i64*** %pvi64, align 8
  %tmpexpr3 = load i8*, i8** %vu8, align 8
  %3 = bitcast i8* %tmpexpr3 to i8**
  store volatile i8** %3, i8*** %pvu8, align 8
  %tmpexpr4 = load i32*, i32** %vu32, align 8
  %4 = bitcast i32* %tmpexpr4 to i32**
  store volatile i32** %4, i32*** %pvu32, align 8
  %tmpexpr5 = load i64*, i64** %vu64, align 8
  %5 = bitcast i64* %tmpexpr5 to i64**
  store volatile i64** %5, i64*** %pvu64, align 8
  %tmpexpr6 = load float*, float** %vf32, align 8
  %6 = bitcast float* %tmpexpr6 to float**
  store volatile float** %6, float*** %pvf32, align 8
  %tmpexpr7 = load double*, double** %vf64, align 8
  %7 = bitcast double* %tmpexpr7 to double**
  store volatile double** %7, double*** %pvf64, align 8
  %tmpexpr8 = load i64*, i64** %visize, align 8
  %8 = bitcast i64* %tmpexpr8 to i64**
  store volatile i64** %8, i64*** %pvisize, align 8
  %tmpexpr9 = load i64*, i64** %vusize, align 8
  %9 = bitcast i64* %tmpexpr9 to i64**
  store volatile i64** %9, i64*** %pvusize, align 8
  %tmpexpr10 = load void*, void** %vvoid, align 8
  %10 = bitcast void* %tmpexpr10 to void**
  store volatile void** %10, void*** %pvvoid, align 8
  %tmpexpr11 = load i8*, i8** %vchar, align 8
  %11 = bitcast i8* %tmpexpr11 to i8**
  store volatile i8** %11, i8*** %pvchar, align 8
  %tmpexpr12 = load i8*, i8** %vuchar, align 8
  %12 = bitcast i8* %tmpexpr12 to i8**
  store volatile i8** %12, i8*** %pvuchar, align 8
  %tmpexpr13 = load i1*, i1** %vbool, align 8
  %13 = bitcast i1* %tmpexpr13 to i1**
  store volatile i1** %13, i1*** %pvbool, align 8
  %tmpexpr14 = load i8*, i8** %vi8, align 8
  %14 = bitcast i8* %tmpexpr14 to i8**
  store volatile i8** %14, i8*** %p2vi8, align 8
  %tmpexpr15 = load i32*, i32** %vi32, align 8
  %15 = bitcast i32* %tmpexpr15 to i32**
  store volatile i32** %15, i32*** %p2vi32, align 8
  %tmpexpr16 = load i64*, i64** %vi64, align 8
  %16 = bitcast i64* %tmpexpr16 to i64**
  store volatile i64** %16, i64*** %p2vi64, align 8
  %tmpexpr17 = load i8*, i8** %vu8, align 8
  %17 = bitcast i8* %tmpexpr17 to i8**
  store volatile i8** %17, i8*** %p2vu8, align 8
  %tmpexpr18 = load i32*, i32** %vu32, align 8
  %18 = bitcast i32* %tmpexpr18 to i32**
  store volatile i32** %18, i32*** %p2vu32, align 8
  %tmpexpr19 = load i64*, i64** %vu64, align 8
  %19 = bitcast i64* %tmpexpr19 to i64**
  store volatile i64** %19, i64*** %p2vu64, align 8
  %tmpexpr20 = load float*, float** %vf32, align 8
  %20 = bitcast float* %tmpexpr20 to float**
  store volatile float** %20, float*** %p2vf32, align 8
  %tmpexpr21 = load double*, double** %vf64, align 8
  %21 = bitcast double* %tmpexpr21 to double**
  store volatile double** %21, double*** %p2vf64, align 8
  %tmpexpr22 = load i64*, i64** %visize, align 8
  %22 = bitcast i64* %tmpexpr22 to i64**
  store volatile i64** %22, i64*** %p2visize, align 8
  %tmpexpr23 = load i64*, i64** %vusize, align 8
  %23 = bitcast i64* %tmpexpr23 to i64**
  store volatile i64** %23, i64*** %p2vusize, align 8
  %tmpexpr24 = load void*, void** %vvoid, align 8
  %24 = bitcast void* %tmpexpr24 to void**
  store volatile void** %24, void*** %p2vvoid, align 8
  %tmpexpr25 = load i8*, i8** %vchar, align 8
  %25 = bitcast i8* %tmpexpr25 to i8**
  store volatile i8** %25, i8*** %p2vchar, align 8
  %tmpexpr26 = load i8*, i8** %vuchar, align 8
  %26 = bitcast i8* %tmpexpr26 to i8**
  store volatile i8** %26, i8*** %p2vuchar, align 8
  %tmpexpr27 = load i1*, i1** %vbool, align 8
  %27 = bitcast i1* %tmpexpr27 to i1**
  store volatile i1** %27, i1*** %p2vbool, align 8
  %tmpexpr28 = load i8*, i8** %vi8, align 8
  %28 = bitcast i8* %tmpexpr28 to i8***
  store volatile i8*** %28, i8**** %ppvi8, align 8
  %tmpexpr29 = load i32*, i32** %vi32, align 8
  %29 = bitcast i32* %tmpexpr29 to i32***
  store volatile i32*** %29, i32**** %ppvi32, align 8
  %tmpexpr30 = load i64*, i64** %vi64, align 8
  %30 = bitcast i64* %tmpexpr30 to i64***
  store volatile i64*** %30, i64**** %ppvi64, align 8
  %tmpexpr31 = load i8*, i8** %vu8, align 8
  %31 = bitcast i8* %tmpexpr31 to i8***
  store volatile i8*** %31, i8**** %ppvu8, align 8
  %tmpexpr32 = load i32*, i32** %vu32, align 8
  %32 = bitcast i32* %tmpexpr32 to i32***
  store volatile i32*** %32, i32**** %ppvu32, align 8
  %tmpexpr33 = load i64*, i64** %vu64, align 8
  %33 = bitcast i64* %tmpexpr33 to i64***
  store volatile i64*** %33, i64**** %ppvu64, align 8
  %tmpexpr34 = load float*, float** %vf32, align 8
  %34 = bitcast float* %tmpexpr34 to float***
  store volatile float*** %34, float**** %ppvf32, align 8
  %tmpexpr35 = load double*, double** %vf64, align 8
  %35 = bitcast double* %tmpexpr35 to double***
  store volatile double*** %35, double**** %ppvf64, align 8
  %tmpexpr36 = load i64*, i64** %visize, align 8
  %36 = bitcast i64* %tmpexpr36 to i64***
  store volatile i64*** %36, i64**** %ppvisize, align 8
  %tmpexpr37 = load i64*, i64** %vusize, align 8
  %37 = bitcast i64* %tmpexpr37 to i64***
  store volatile i64*** %37, i64**** %ppvusize, align 8
  %tmpexpr38 = load void*, void** %vvoid, align 8
  %38 = bitcast void* %tmpexpr38 to void***
  store volatile void*** %38, void**** %ppvvoid, align 8
  %tmpexpr39 = load i8*, i8** %vchar, align 8
  %39 = bitcast i8* %tmpexpr39 to i8***
  store volatile i8*** %39, i8**** %ppvchar, align 8
  %tmpexpr40 = load i8*, i8** %vuchar, align 8
  %40 = bitcast i8* %tmpexpr40 to i8***
  store volatile i8*** %40, i8**** %ppvuchar, align 8
  %tmpexpr41 = load i1*, i1** %vbool, align 8
  %41 = bitcast i1* %tmpexpr41 to i1***
  store volatile i1*** %41, i1**** %ppvbool, align 8
  %tmpexpr42 = load i8*, i8** %vi8, align 8
  %42 = bitcast i8* %tmpexpr42 to i8***
  store volatile i8*** %42, i8**** %pp2vi8, align 8
  %tmpexpr43 = load i32*, i32** %vi32, align 8
  %43 = bitcast i32* %tmpexpr43 to i32***
  store volatile i32*** %43, i32**** %pp2vi32, align 8
  %tmpexpr44 = load i64*, i64** %vi64, align 8
  %44 = bitcast i64* %tmpexpr44 to i64***
  store volatile i64*** %44, i64**** %pp2vi64, align 8
  %tmpexpr45 = load i8*, i8** %vu8, align 8
  %45 = bitcast i8* %tmpexpr45 to i8***
  store volatile i8*** %45, i8**** %pp2vu8, align 8
  %tmpexpr46 = load i32*, i32** %vu32, align 8
  %46 = bitcast i32* %tmpexpr46 to i32***
  store volatile i32*** %46, i32**** %pp2vu32, align 8
  %tmpexpr47 = load i64*, i64** %vu64, align 8
  %47 = bitcast i64* %tmpexpr47 to i64***
  store volatile i64*** %47, i64**** %pp2vu64, align 8
  %tmpexpr48 = load float*, float** %vf32, align 8
  %48 = bitcast float* %tmpexpr48 to float***
  store volatile float*** %48, float**** %pp2vf32, align 8
  %tmpexpr49 = load double*, double** %vf64, align 8
  %49 = bitcast double* %tmpexpr49 to double***
  store volatile double*** %49, double**** %pp2vf64, align 8
  %tmpexpr50 = load i64*, i64** %visize, align 8
  %50 = bitcast i64* %tmpexpr50 to i64***
  store volatile i64*** %50, i64**** %pp2visize, align 8
  %tmpexpr51 = load i64*, i64** %vusize, align 8
  %51 = bitcast i64* %tmpexpr51 to i64***
  store volatile i64*** %51, i64**** %pp2vusize, align 8
  %tmpexpr52 = load void*, void** %vvoid, align 8
  %52 = bitcast void* %tmpexpr52 to void***
  store volatile void*** %52, void**** %pp2vvoid, align 8
  %tmpexpr53 = load i8*, i8** %vchar, align 8
  %53 = bitcast i8* %tmpexpr53 to i8***
  store volatile i8*** %53, i8**** %pp2vchar, align 8
  %tmpexpr54 = load i8*, i8** %vuchar, align 8
  %54 = bitcast i8* %tmpexpr54 to i8***
  store volatile i8*** %54, i8**** %pp2vuchar, align 8
  %tmpexpr55 = load i1*, i1** %vbool, align 8
  %55 = bitcast i1* %tmpexpr55 to i1***
  store volatile i1*** %55, i1**** %pp2vbool, align 8
  %tmpexpr56 = load i8*, i8** %vi8, align 8
  %56 = bitcast i8* %tmpexpr56 to i8****
  store volatile i8**** %56, i8***** %pppvi8, align 8
  %tmpexpr57 = load i32*, i32** %vi32, align 8
  %57 = bitcast i32* %tmpexpr57 to i32****
  store volatile i32**** %57, i32***** %pppvi32, align 8
  %tmpexpr58 = load i64*, i64** %vi64, align 8
  %58 = bitcast i64* %tmpexpr58 to i64****
  store volatile i64**** %58, i64***** %pppvi64, align 8
  %tmpexpr59 = load i8*, i8** %vu8, align 8
  %59 = bitcast i8* %tmpexpr59 to i8****
  store volatile i8**** %59, i8***** %pppvu8, align 8
  %tmpexpr60 = load i32*, i32** %vu32, align 8
  %60 = bitcast i32* %tmpexpr60 to i32****
  store volatile i32**** %60, i32***** %pppvu32, align 8
  %tmpexpr61 = load i64*, i64** %vu64, align 8
  %61 = bitcast i64* %tmpexpr61 to i64****
  store volatile i64**** %61, i64***** %pppvu64, align 8
  %tmpexpr62 = load float*, float** %vf32, align 8
  %62 = bitcast float* %tmpexpr62 to float****
  store volatile float**** %62, float***** %pppvf32, align 8
  %tmpexpr63 = load double*, double** %vf64, align 8
  %63 = bitcast double* %tmpexpr63 to double****
  store volatile double**** %63, double***** %pppvf64, align 8
  %tmpexpr64 = load i64*, i64** %visize, align 8
  %64 = bitcast i64* %tmpexpr64 to i64****
  store volatile i64**** %64, i64***** %pppvisize, align 8
  %tmpexpr65 = load i64*, i64** %vusize, align 8
  %65 = bitcast i64* %tmpexpr65 to i64****
  store volatile i64**** %65, i64***** %pppvusize, align 8
  %tmpexpr66 = load void*, void** %vvoid, align 8
  %66 = bitcast void* %tmpexpr66 to void****
  store volatile void**** %66, void***** %pppvvoid, align 8
  %tmpexpr67 = load i8*, i8** %vchar, align 8
  %67 = bitcast i8* %tmpexpr67 to i8****
  store volatile i8**** %67, i8***** %pppvchar, align 8
  %tmpexpr68 = load i8*, i8** %vuchar, align 8
  %68 = bitcast i8* %tmpexpr68 to i8****
  store volatile i8**** %68, i8***** %pppvuchar, align 8
  %tmpexpr69 = load i1*, i1** %vbool, align 8
  %69 = bitcast i1* %tmpexpr69 to i1****
  store volatile i1**** %69, i1***** %pppvbool, align 8
  %tmpexpr70 = load i8*, i8** %vi8, align 8
  %70 = bitcast i8* %tmpexpr70 to i8****
  store volatile i8**** %70, i8***** %ppp2vi8, align 8
  %tmpexpr71 = load i32*, i32** %vi32, align 8
  %71 = bitcast i32* %tmpexpr71 to i32****
  store volatile i32**** %71, i32***** %ppp2vi32, align 8
  %tmpexpr72 = load i64*, i64** %vi64, align 8
  %72 = bitcast i64* %tmpexpr72 to i64****
  store volatile i64**** %72, i64***** %ppp2vi64, align 8
  %tmpexpr73 = load i8*, i8** %vu8, align 8
  %73 = bitcast i8* %tmpexpr73 to i8****
  store volatile i8**** %73, i8***** %ppp2vu8, align 8
  %tmpexpr74 = load i32*, i32** %vu32, align 8
  %74 = bitcast i32* %tmpexpr74 to i32****
  store volatile i32**** %74, i32***** %ppp2vu32, align 8
  %tmpexpr75 = load i64*, i64** %vu64, align 8
  %75 = bitcast i64* %tmpexpr75 to i64****
  store volatile i64**** %75, i64***** %ppp2vu64, align 8
  %tmpexpr76 = load float*, float** %vf32, align 8
  %76 = bitcast float* %tmpexpr76 to float****
  store volatile float**** %76, float***** %ppp2vf32, align 8
  %tmpexpr77 = load double*, double** %vf64, align 8
  %77 = bitcast double* %tmpexpr77 to double****
  store volatile double**** %77, double***** %ppp2vf64, align 8
  %tmpexpr78 = load i64*, i64** %visize, align 8
  %78 = bitcast i64* %tmpexpr78 to i64****
  store volatile i64**** %78, i64***** %ppp2visize, align 8
  %tmpexpr79 = load i64*, i64** %vusize, align 8
  %79 = bitcast i64* %tmpexpr79 to i64****
  store volatile i64**** %79, i64***** %ppp2vusize, align 8
  %tmpexpr80 = load void*, void** %vvoid, align 8
  %80 = bitcast void* %tmpexpr80 to void****
  store volatile void**** %80, void***** %ppp2vvoid, align 8
  %tmpexpr81 = load i8*, i8** %vchar, align 8
  %81 = bitcast i8* %tmpexpr81 to i8****
  store volatile i8**** %81, i8***** %ppp2vchar, align 8
  %tmpexpr82 = load i8*, i8** %vuchar, align 8
  %82 = bitcast i8* %tmpexpr82 to i8****
  store volatile i8**** %82, i8***** %ppp2vuchar, align 8
  %tmpexpr83 = load i1*, i1** %vbool, align 8
  %83 = bitcast i1* %tmpexpr83 to i1****
  store volatile i1**** %83, i1***** %ppp2vbool, align 8
  %tmpexpr84 = load i8****, i8***** %ppp2vi8, align 8
  %84 = bitcast i8**** %tmpexpr84 to i8*
  store volatile i8* %84, i8** %svi8, align 8
  %tmpexpr85 = load i32****, i32***** %ppp2vi32, align 8
  %85 = bitcast i32**** %tmpexpr85 to i32*
  store volatile i32* %85, i32** %svi32, align 8
  %tmpexpr86 = load i64****, i64***** %ppp2vi64, align 8
  %86 = bitcast i64**** %tmpexpr86 to i64*
  store volatile i64* %86, i64** %svi64, align 8
  %tmpexpr87 = load i8****, i8***** %ppp2vu8, align 8
  %87 = bitcast i8**** %tmpexpr87 to i8*
  store volatile i8* %87, i8** %svu8, align 8
  %tmpexpr88 = load i32****, i32***** %ppp2vu32, align 8
  %88 = bitcast i32**** %tmpexpr88 to i32*
  store volatile i32* %88, i32** %svu32, align 8
  %tmpexpr89 = load i64****, i64***** %ppp2vu64, align 8
  %89 = bitcast i64**** %tmpexpr89 to i64*
  store volatile i64* %89, i64** %svu64, align 8
  %tmpexpr90 = load float****, float***** %ppp2vf32, align 8
  %90 = bitcast float**** %tmpexpr90 to float*
  store volatile float* %90, float** %svf32, align 8
  %tmpexpr91 = load double****, double***** %ppp2vf64, align 8
  %91 = bitcast double**** %tmpexpr91 to double*
  store volatile double* %91, double** %svf64, align 8
  %tmpexpr92 = load i64****, i64***** %ppp2visize, align 8
  %92 = bitcast i64**** %tmpexpr92 to i64*
  store volatile i64* %92, i64** %svisize, align 8
  %tmpexpr93 = load i64****, i64***** %ppp2vusize, align 8
  %93 = bitcast i64**** %tmpexpr93 to i64*
  store volatile i64* %93, i64** %svusize, align 8
  %tmpexpr94 = load void****, void***** %ppp2vvoid, align 8
  %94 = bitcast void**** %tmpexpr94 to void*
  store volatile void* %94, void** %svvoid, align 8
  %tmpexpr95 = load i8****, i8***** %ppp2vchar, align 8
  %95 = bitcast i8**** %tmpexpr95 to i8*
  store volatile i8* %95, i8** %svchar, align 8
  %tmpexpr96 = load i8****, i8***** %ppp2vuchar, align 8
  %96 = bitcast i8**** %tmpexpr96 to i8*
  store volatile i8* %96, i8** %svuchar, align 8
  %tmpexpr97 = load i1****, i1***** %ppp2vbool, align 8
  %97 = bitcast i1**** %tmpexpr97 to i1*
  store volatile i1* %97, i1** %svbool, align 8
  br label %ret

ret:                                              ; preds = %entry
  ret void
}
