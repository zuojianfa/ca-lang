; ModuleID = 'pointer_mixed2.ca'
source_filename = "pointer_mixed2.ca"

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
  %ppp2vbool = alloca i64****, align 8
  %ppp2vuchar = alloca i32****, align 8
  %ppp2vchar = alloca i8****, align 8
  %ppp2vvoid = alloca i64****, align 8
  %ppp2vusize = alloca i32****, align 8
  %ppp2visize = alloca i8****, align 8
  %ppp2vf64 = alloca i1****, align 8
  %ppp2vf32 = alloca i8****, align 8
  %ppp2vu64 = alloca i8****, align 8
  %ppp2vu32 = alloca void****, align 8
  %ppp2vu8 = alloca i64****, align 8
  %ppp2vi64 = alloca i64****, align 8
  %ppp2vi32 = alloca double****, align 8
  %ppp2vi8 = alloca float****, align 8
  %pppvbool = alloca i64****, align 8
  %pppvuchar = alloca i32****, align 8
  %pppvchar = alloca i8****, align 8
  %pppvvoid = alloca i64****, align 8
  %pppvusize = alloca i32****, align 8
  %pppvisize = alloca i8****, align 8
  %pppvf64 = alloca i1****, align 8
  %pppvf32 = alloca i8****, align 8
  %pppvu64 = alloca i8****, align 8
  %pppvu32 = alloca void****, align 8
  %pppvu8 = alloca i64****, align 8
  %pppvi64 = alloca i64****, align 8
  %pppvi32 = alloca double****, align 8
  %pppvi8 = alloca float****, align 8
  %pp2vbool = alloca i64***, align 8
  %pp2vuchar = alloca i32***, align 8
  %pp2vchar = alloca i8***, align 8
  %pp2vvoid = alloca i64***, align 8
  %pp2vusize = alloca i32***, align 8
  %pp2visize = alloca i8***, align 8
  %pp2vf64 = alloca i1***, align 8
  %pp2vf32 = alloca i8***, align 8
  %pp2vu64 = alloca i8***, align 8
  %pp2vu32 = alloca void***, align 8
  %pp2vu8 = alloca i64***, align 8
  %pp2vi64 = alloca i64***, align 8
  %pp2vi32 = alloca double***, align 8
  %pp2vi8 = alloca float***, align 8
  %ppvbool = alloca i64***, align 8
  %ppvuchar = alloca i32***, align 8
  %ppvchar = alloca i8***, align 8
  %ppvvoid = alloca i64***, align 8
  %ppvusize = alloca i32***, align 8
  %ppvisize = alloca i8***, align 8
  %ppvf64 = alloca i1***, align 8
  %ppvf32 = alloca i8***, align 8
  %ppvu64 = alloca i8***, align 8
  %ppvu32 = alloca void***, align 8
  %ppvu8 = alloca i64***, align 8
  %ppvi64 = alloca i64***, align 8
  %ppvi32 = alloca double***, align 8
  %ppvi8 = alloca float***, align 8
  %p2vbool = alloca i64**, align 8
  %p2vuchar = alloca i32**, align 8
  %p2vchar = alloca i8**, align 8
  %p2vvoid = alloca i64**, align 8
  %p2vusize = alloca i32**, align 8
  %p2visize = alloca i8**, align 8
  %p2vf64 = alloca i1**, align 8
  %p2vf32 = alloca i8**, align 8
  %p2vu64 = alloca i8**, align 8
  %p2vu32 = alloca void**, align 8
  %p2vu8 = alloca i64**, align 8
  %p2vi64 = alloca i64**, align 8
  %p2vi32 = alloca double**, align 8
  %p2vi8 = alloca float**, align 8
  %pvbool = alloca i64**, align 8
  %pvuchar = alloca i32**, align 8
  %pvchar = alloca i8**, align 8
  %pvvoid = alloca i64**, align 8
  %pvusize = alloca i32**, align 8
  %pvisize = alloca i8**, align 8
  %pvf64 = alloca i1**, align 8
  %pvf32 = alloca i8**, align 8
  %pvu64 = alloca i8**, align 8
  %pvu32 = alloca void**, align 8
  %pvu8 = alloca i64**, align 8
  %pvi64 = alloca i64**, align 8
  %pvi32 = alloca double**, align 8
  %pvi8 = alloca float**, align 8
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
  %a = alloca i32, align 4
  store volatile i32 1234, i32* %a, align 4
  %tmpexpr = load i32, i32* %a, align 4
  %0 = inttoptr i32 %tmpexpr to i8*
  store volatile i8* %0, i8** %vi8, align 8
  %tmpexpr1 = load i32, i32* %a, align 4
  %1 = inttoptr i32 %tmpexpr1 to i32*
  store volatile i32* %1, i32** %vi32, align 8
  %tmpexpr2 = load i32, i32* %a, align 4
  %2 = inttoptr i32 %tmpexpr2 to i64*
  store volatile i64* %2, i64** %vi64, align 8
  %tmpexpr3 = load i32, i32* %a, align 4
  %3 = inttoptr i32 %tmpexpr3 to i8*
  store volatile i8* %3, i8** %vu8, align 8
  %tmpexpr4 = load i32, i32* %a, align 4
  %4 = inttoptr i32 %tmpexpr4 to i32*
  store volatile i32* %4, i32** %vu32, align 8
  %tmpexpr5 = load i32, i32* %a, align 4
  %5 = inttoptr i32 %tmpexpr5 to i64*
  store volatile i64* %5, i64** %vu64, align 8
  %tmpexpr6 = load i32, i32* %a, align 4
  %6 = inttoptr i32 %tmpexpr6 to float*
  store volatile float* %6, float** %vf32, align 8
  %tmpexpr7 = load i32, i32* %a, align 4
  %7 = inttoptr i32 %tmpexpr7 to double*
  store volatile double* %7, double** %vf64, align 8
  %tmpexpr8 = load i32, i32* %a, align 4
  %8 = inttoptr i32 %tmpexpr8 to i64*
  store volatile i64* %8, i64** %visize, align 8
  %tmpexpr9 = load i32, i32* %a, align 4
  %9 = inttoptr i32 %tmpexpr9 to i64*
  store volatile i64* %9, i64** %vusize, align 8
  %tmpexpr10 = load i32, i32* %a, align 4
  %10 = inttoptr i32 %tmpexpr10 to void*
  store volatile void* %10, void** %vvoid, align 8
  %tmpexpr11 = load i32, i32* %a, align 4
  %11 = inttoptr i32 %tmpexpr11 to i8*
  store volatile i8* %11, i8** %vchar, align 8
  %tmpexpr12 = load i32, i32* %a, align 4
  %12 = inttoptr i32 %tmpexpr12 to i8*
  store volatile i8* %12, i8** %vuchar, align 8
  %tmpexpr13 = load i32, i32* %a, align 4
  %13 = inttoptr i32 %tmpexpr13 to i1*
  store volatile i1* %13, i1** %vbool, align 8
  %tmpexpr14 = load i8*, i8** %vi8, align 8
  %14 = bitcast i8* %tmpexpr14 to float**
  store volatile float** %14, float*** %pvi8, align 8
  %tmpexpr15 = load i32*, i32** %vi32, align 8
  %15 = bitcast i32* %tmpexpr15 to double**
  store volatile double** %15, double*** %pvi32, align 8
  %tmpexpr16 = load i64*, i64** %vi64, align 8
  %16 = bitcast i64* %tmpexpr16 to i64**
  store volatile i64** %16, i64*** %pvi64, align 8
  %tmpexpr17 = load i8*, i8** %vu8, align 8
  %17 = bitcast i8* %tmpexpr17 to i64**
  store volatile i64** %17, i64*** %pvu8, align 8
  %tmpexpr18 = load i32*, i32** %vu32, align 8
  %18 = bitcast i32* %tmpexpr18 to void**
  store volatile void** %18, void*** %pvu32, align 8
  %tmpexpr19 = load i64*, i64** %vu64, align 8
  %19 = bitcast i64* %tmpexpr19 to i8**
  store volatile i8** %19, i8*** %pvu64, align 8
  %tmpexpr20 = load float*, float** %vf32, align 8
  %20 = bitcast float* %tmpexpr20 to i8**
  store volatile i8** %20, i8*** %pvf32, align 8
  %tmpexpr21 = load double*, double** %vf64, align 8
  %21 = bitcast double* %tmpexpr21 to i1**
  store volatile i1** %21, i1*** %pvf64, align 8
  %tmpexpr22 = load i64*, i64** %visize, align 8
  %22 = bitcast i64* %tmpexpr22 to i8**
  store volatile i8** %22, i8*** %pvisize, align 8
  %tmpexpr23 = load i64*, i64** %vusize, align 8
  %23 = bitcast i64* %tmpexpr23 to i32**
  store volatile i32** %23, i32*** %pvusize, align 8
  %tmpexpr24 = load void*, void** %vvoid, align 8
  %24 = bitcast void* %tmpexpr24 to i64**
  store volatile i64** %24, i64*** %pvvoid, align 8
  %tmpexpr25 = load i8*, i8** %vchar, align 8
  %25 = bitcast i8* %tmpexpr25 to i8**
  store volatile i8** %25, i8*** %pvchar, align 8
  %tmpexpr26 = load i8*, i8** %vuchar, align 8
  %26 = bitcast i8* %tmpexpr26 to i32**
  store volatile i32** %26, i32*** %pvuchar, align 8
  %tmpexpr27 = load i1*, i1** %vbool, align 8
  %27 = bitcast i1* %tmpexpr27 to i64**
  store volatile i64** %27, i64*** %pvbool, align 8
  %tmpexpr28 = load i8*, i8** %vi8, align 8
  %28 = bitcast i8* %tmpexpr28 to float**
  store volatile float** %28, float*** %p2vi8, align 8
  %tmpexpr29 = load i32*, i32** %vi32, align 8
  %29 = bitcast i32* %tmpexpr29 to double**
  store volatile double** %29, double*** %p2vi32, align 8
  %tmpexpr30 = load i64*, i64** %vi64, align 8
  %30 = bitcast i64* %tmpexpr30 to i64**
  store volatile i64** %30, i64*** %p2vi64, align 8
  %tmpexpr31 = load i8*, i8** %vu8, align 8
  %31 = bitcast i8* %tmpexpr31 to i64**
  store volatile i64** %31, i64*** %p2vu8, align 8
  %tmpexpr32 = load i32*, i32** %vu32, align 8
  %32 = bitcast i32* %tmpexpr32 to void**
  store volatile void** %32, void*** %p2vu32, align 8
  %tmpexpr33 = load i64*, i64** %vu64, align 8
  %33 = bitcast i64* %tmpexpr33 to i8**
  store volatile i8** %33, i8*** %p2vu64, align 8
  %tmpexpr34 = load float*, float** %vf32, align 8
  %34 = bitcast float* %tmpexpr34 to i8**
  store volatile i8** %34, i8*** %p2vf32, align 8
  %tmpexpr35 = load double*, double** %vf64, align 8
  %35 = bitcast double* %tmpexpr35 to i1**
  store volatile i1** %35, i1*** %p2vf64, align 8
  %tmpexpr36 = load i64*, i64** %visize, align 8
  %36 = bitcast i64* %tmpexpr36 to i8**
  store volatile i8** %36, i8*** %p2visize, align 8
  %tmpexpr37 = load i64*, i64** %vusize, align 8
  %37 = bitcast i64* %tmpexpr37 to i32**
  store volatile i32** %37, i32*** %p2vusize, align 8
  %tmpexpr38 = load void*, void** %vvoid, align 8
  %38 = bitcast void* %tmpexpr38 to i64**
  store volatile i64** %38, i64*** %p2vvoid, align 8
  %tmpexpr39 = load i8*, i8** %vchar, align 8
  %39 = bitcast i8* %tmpexpr39 to i8**
  store volatile i8** %39, i8*** %p2vchar, align 8
  %tmpexpr40 = load i8*, i8** %vuchar, align 8
  %40 = bitcast i8* %tmpexpr40 to i32**
  store volatile i32** %40, i32*** %p2vuchar, align 8
  %tmpexpr41 = load i1*, i1** %vbool, align 8
  %41 = bitcast i1* %tmpexpr41 to i64**
  store volatile i64** %41, i64*** %p2vbool, align 8
  %tmpexpr42 = load i8*, i8** %vi8, align 8
  %42 = bitcast i8* %tmpexpr42 to float***
  store volatile float*** %42, float**** %ppvi8, align 8
  %tmpexpr43 = load i32*, i32** %vi32, align 8
  %43 = bitcast i32* %tmpexpr43 to double***
  store volatile double*** %43, double**** %ppvi32, align 8
  %tmpexpr44 = load i64*, i64** %vi64, align 8
  %44 = bitcast i64* %tmpexpr44 to i64***
  store volatile i64*** %44, i64**** %ppvi64, align 8
  %tmpexpr45 = load i8*, i8** %vu8, align 8
  %45 = bitcast i8* %tmpexpr45 to i64***
  store volatile i64*** %45, i64**** %ppvu8, align 8
  %tmpexpr46 = load i32*, i32** %vu32, align 8
  %46 = bitcast i32* %tmpexpr46 to void***
  store volatile void*** %46, void**** %ppvu32, align 8
  %tmpexpr47 = load i64*, i64** %vu64, align 8
  %47 = bitcast i64* %tmpexpr47 to i8***
  store volatile i8*** %47, i8**** %ppvu64, align 8
  %tmpexpr48 = load float*, float** %vf32, align 8
  %48 = bitcast float* %tmpexpr48 to i8***
  store volatile i8*** %48, i8**** %ppvf32, align 8
  %tmpexpr49 = load double*, double** %vf64, align 8
  %49 = bitcast double* %tmpexpr49 to i1***
  store volatile i1*** %49, i1**** %ppvf64, align 8
  %tmpexpr50 = load i64*, i64** %visize, align 8
  %50 = bitcast i64* %tmpexpr50 to i8***
  store volatile i8*** %50, i8**** %ppvisize, align 8
  %tmpexpr51 = load i64*, i64** %vusize, align 8
  %51 = bitcast i64* %tmpexpr51 to i32***
  store volatile i32*** %51, i32**** %ppvusize, align 8
  %tmpexpr52 = load void*, void** %vvoid, align 8
  %52 = bitcast void* %tmpexpr52 to i64***
  store volatile i64*** %52, i64**** %ppvvoid, align 8
  %tmpexpr53 = load i8*, i8** %vchar, align 8
  %53 = bitcast i8* %tmpexpr53 to i8***
  store volatile i8*** %53, i8**** %ppvchar, align 8
  %tmpexpr54 = load i8*, i8** %vuchar, align 8
  %54 = bitcast i8* %tmpexpr54 to i32***
  store volatile i32*** %54, i32**** %ppvuchar, align 8
  %tmpexpr55 = load i1*, i1** %vbool, align 8
  %55 = bitcast i1* %tmpexpr55 to i64***
  store volatile i64*** %55, i64**** %ppvbool, align 8
  %tmpexpr56 = load i8*, i8** %vi8, align 8
  %56 = bitcast i8* %tmpexpr56 to float***
  store volatile float*** %56, float**** %pp2vi8, align 8
  %tmpexpr57 = load i32*, i32** %vi32, align 8
  %57 = bitcast i32* %tmpexpr57 to double***
  store volatile double*** %57, double**** %pp2vi32, align 8
  %tmpexpr58 = load i64*, i64** %vi64, align 8
  %58 = bitcast i64* %tmpexpr58 to i64***
  store volatile i64*** %58, i64**** %pp2vi64, align 8
  %tmpexpr59 = load i8*, i8** %vu8, align 8
  %59 = bitcast i8* %tmpexpr59 to i64***
  store volatile i64*** %59, i64**** %pp2vu8, align 8
  %tmpexpr60 = load i32*, i32** %vu32, align 8
  %60 = bitcast i32* %tmpexpr60 to void***
  store volatile void*** %60, void**** %pp2vu32, align 8
  %tmpexpr61 = load i64*, i64** %vu64, align 8
  %61 = bitcast i64* %tmpexpr61 to i8***
  store volatile i8*** %61, i8**** %pp2vu64, align 8
  %tmpexpr62 = load float*, float** %vf32, align 8
  %62 = bitcast float* %tmpexpr62 to i8***
  store volatile i8*** %62, i8**** %pp2vf32, align 8
  %tmpexpr63 = load double*, double** %vf64, align 8
  %63 = bitcast double* %tmpexpr63 to i1***
  store volatile i1*** %63, i1**** %pp2vf64, align 8
  %tmpexpr64 = load i64*, i64** %visize, align 8
  %64 = bitcast i64* %tmpexpr64 to i8***
  store volatile i8*** %64, i8**** %pp2visize, align 8
  %tmpexpr65 = load i64*, i64** %vusize, align 8
  %65 = bitcast i64* %tmpexpr65 to i32***
  store volatile i32*** %65, i32**** %pp2vusize, align 8
  %tmpexpr66 = load void*, void** %vvoid, align 8
  %66 = bitcast void* %tmpexpr66 to i64***
  store volatile i64*** %66, i64**** %pp2vvoid, align 8
  %tmpexpr67 = load i8*, i8** %vchar, align 8
  %67 = bitcast i8* %tmpexpr67 to i8***
  store volatile i8*** %67, i8**** %pp2vchar, align 8
  %tmpexpr68 = load i8*, i8** %vuchar, align 8
  %68 = bitcast i8* %tmpexpr68 to i32***
  store volatile i32*** %68, i32**** %pp2vuchar, align 8
  %tmpexpr69 = load i1*, i1** %vbool, align 8
  %69 = bitcast i1* %tmpexpr69 to i64***
  store volatile i64*** %69, i64**** %pp2vbool, align 8
  %tmpexpr70 = load i8*, i8** %vi8, align 8
  %70 = bitcast i8* %tmpexpr70 to float****
  store volatile float**** %70, float***** %pppvi8, align 8
  %tmpexpr71 = load i32*, i32** %vi32, align 8
  %71 = bitcast i32* %tmpexpr71 to double****
  store volatile double**** %71, double***** %pppvi32, align 8
  %tmpexpr72 = load i64*, i64** %vi64, align 8
  %72 = bitcast i64* %tmpexpr72 to i64****
  store volatile i64**** %72, i64***** %pppvi64, align 8
  %tmpexpr73 = load i8*, i8** %vu8, align 8
  %73 = bitcast i8* %tmpexpr73 to i64****
  store volatile i64**** %73, i64***** %pppvu8, align 8
  %tmpexpr74 = load i32*, i32** %vu32, align 8
  %74 = bitcast i32* %tmpexpr74 to void****
  store volatile void**** %74, void***** %pppvu32, align 8
  %tmpexpr75 = load i64*, i64** %vu64, align 8
  %75 = bitcast i64* %tmpexpr75 to i8****
  store volatile i8**** %75, i8***** %pppvu64, align 8
  %tmpexpr76 = load float*, float** %vf32, align 8
  %76 = bitcast float* %tmpexpr76 to i8****
  store volatile i8**** %76, i8***** %pppvf32, align 8
  %tmpexpr77 = load double*, double** %vf64, align 8
  %77 = bitcast double* %tmpexpr77 to i1****
  store volatile i1**** %77, i1***** %pppvf64, align 8
  %tmpexpr78 = load i64*, i64** %visize, align 8
  %78 = bitcast i64* %tmpexpr78 to i8****
  store volatile i8**** %78, i8***** %pppvisize, align 8
  %tmpexpr79 = load i64*, i64** %vusize, align 8
  %79 = bitcast i64* %tmpexpr79 to i32****
  store volatile i32**** %79, i32***** %pppvusize, align 8
  %tmpexpr80 = load void*, void** %vvoid, align 8
  %80 = bitcast void* %tmpexpr80 to i64****
  store volatile i64**** %80, i64***** %pppvvoid, align 8
  %tmpexpr81 = load i8*, i8** %vchar, align 8
  %81 = bitcast i8* %tmpexpr81 to i8****
  store volatile i8**** %81, i8***** %pppvchar, align 8
  %tmpexpr82 = load i8*, i8** %vuchar, align 8
  %82 = bitcast i8* %tmpexpr82 to i32****
  store volatile i32**** %82, i32***** %pppvuchar, align 8
  %tmpexpr83 = load i1*, i1** %vbool, align 8
  %83 = bitcast i1* %tmpexpr83 to i64****
  store volatile i64**** %83, i64***** %pppvbool, align 8
  %tmpexpr84 = load i8*, i8** %vi8, align 8
  %84 = bitcast i8* %tmpexpr84 to float****
  store volatile float**** %84, float***** %ppp2vi8, align 8
  %tmpexpr85 = load i32*, i32** %vi32, align 8
  %85 = bitcast i32* %tmpexpr85 to double****
  store volatile double**** %85, double***** %ppp2vi32, align 8
  %tmpexpr86 = load i64*, i64** %vi64, align 8
  %86 = bitcast i64* %tmpexpr86 to i64****
  store volatile i64**** %86, i64***** %ppp2vi64, align 8
  %tmpexpr87 = load i8*, i8** %vu8, align 8
  %87 = bitcast i8* %tmpexpr87 to i64****
  store volatile i64**** %87, i64***** %ppp2vu8, align 8
  %tmpexpr88 = load i32*, i32** %vu32, align 8
  %88 = bitcast i32* %tmpexpr88 to void****
  store volatile void**** %88, void***** %ppp2vu32, align 8
  %tmpexpr89 = load i64*, i64** %vu64, align 8
  %89 = bitcast i64* %tmpexpr89 to i8****
  store volatile i8**** %89, i8***** %ppp2vu64, align 8
  %tmpexpr90 = load float*, float** %vf32, align 8
  %90 = bitcast float* %tmpexpr90 to i8****
  store volatile i8**** %90, i8***** %ppp2vf32, align 8
  %tmpexpr91 = load double*, double** %vf64, align 8
  %91 = bitcast double* %tmpexpr91 to i1****
  store volatile i1**** %91, i1***** %ppp2vf64, align 8
  %tmpexpr92 = load i64*, i64** %visize, align 8
  %92 = bitcast i64* %tmpexpr92 to i8****
  store volatile i8**** %92, i8***** %ppp2visize, align 8
  %tmpexpr93 = load i64*, i64** %vusize, align 8
  %93 = bitcast i64* %tmpexpr93 to i32****
  store volatile i32**** %93, i32***** %ppp2vusize, align 8
  %tmpexpr94 = load void*, void** %vvoid, align 8
  %94 = bitcast void* %tmpexpr94 to i64****
  store volatile i64**** %94, i64***** %ppp2vvoid, align 8
  %tmpexpr95 = load i8*, i8** %vchar, align 8
  %95 = bitcast i8* %tmpexpr95 to i8****
  store volatile i8**** %95, i8***** %ppp2vchar, align 8
  %tmpexpr96 = load i8*, i8** %vuchar, align 8
  %96 = bitcast i8* %tmpexpr96 to i32****
  store volatile i32**** %96, i32***** %ppp2vuchar, align 8
  %tmpexpr97 = load i1*, i1** %vbool, align 8
  %97 = bitcast i1* %tmpexpr97 to i64****
  store volatile i64**** %97, i64***** %ppp2vbool, align 8
  %tmpexpr98 = load float****, float***** %ppp2vi8, align 8
  %98 = bitcast float**** %tmpexpr98 to i8*
  store volatile i8* %98, i8** %svi8, align 8
  %tmpexpr99 = load double****, double***** %ppp2vi32, align 8
  %99 = bitcast double**** %tmpexpr99 to i32*
  store volatile i32* %99, i32** %svi32, align 8
  %tmpexpr100 = load i64****, i64***** %ppp2vi64, align 8
  %100 = bitcast i64**** %tmpexpr100 to i64*
  store volatile i64* %100, i64** %svi64, align 8
  %tmpexpr101 = load i64****, i64***** %ppp2vu8, align 8
  %101 = bitcast i64**** %tmpexpr101 to i8*
  store volatile i8* %101, i8** %svu8, align 8
  %tmpexpr102 = load void****, void***** %ppp2vu32, align 8
  %102 = bitcast void**** %tmpexpr102 to i32*
  store volatile i32* %102, i32** %svu32, align 8
  %tmpexpr103 = load i8****, i8***** %ppp2vu64, align 8
  %103 = bitcast i8**** %tmpexpr103 to i64*
  store volatile i64* %103, i64** %svu64, align 8
  %tmpexpr104 = load i8****, i8***** %ppp2vf32, align 8
  %104 = bitcast i8**** %tmpexpr104 to float*
  store volatile float* %104, float** %svf32, align 8
  %tmpexpr105 = load i1****, i1***** %ppp2vf64, align 8
  %105 = bitcast i1**** %tmpexpr105 to double*
  store volatile double* %105, double** %svf64, align 8
  %tmpexpr106 = load i8****, i8***** %ppp2visize, align 8
  %106 = bitcast i8**** %tmpexpr106 to i64*
  store volatile i64* %106, i64** %svisize, align 8
  %tmpexpr107 = load i32****, i32***** %ppp2vusize, align 8
  %107 = bitcast i32**** %tmpexpr107 to i64*
  store volatile i64* %107, i64** %svusize, align 8
  %tmpexpr108 = load i64****, i64***** %ppp2vvoid, align 8
  %108 = bitcast i64**** %tmpexpr108 to void*
  store volatile void* %108, void** %svvoid, align 8
  %tmpexpr109 = load i8****, i8***** %ppp2vchar, align 8
  %109 = bitcast i8**** %tmpexpr109 to i8*
  store volatile i8* %109, i8** %svchar, align 8
  %tmpexpr110 = load i32****, i32***** %ppp2vuchar, align 8
  %110 = bitcast i32**** %tmpexpr110 to i8*
  store volatile i8* %110, i8** %svuchar, align 8
  %tmpexpr111 = load i64****, i64***** %ppp2vbool, align 8
  %111 = bitcast i64**** %tmpexpr111 to i1*
  store volatile i1* %111, i1** %svbool, align 8
  br label %ret

ret:                                              ; preds = %entry
  ret void
}
