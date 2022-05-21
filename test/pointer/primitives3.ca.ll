; ModuleID = 'primitives3.ca'
source_filename = "primitives3.ca"

declare i32 @printf(i8*, ...)

define void @main() {
entry:
  %vasbool = alloca i1***, align 8
  %vasuchar = alloca i8***, align 8
  %vaschar = alloca i8***, align 8
  %vasvoid = alloca void***, align 8
  %vasusize = alloca i64***, align 8
  %vasisize = alloca i64***, align 8
  %vasf64 = alloca double***, align 8
  %vasf32 = alloca float***, align 8
  %vasu64 = alloca i64***, align 8
  %vasu32 = alloca i32***, align 8
  %vasu8 = alloca i8***, align 8
  %vasi64 = alloca i64***, align 8
  %vasi32 = alloca i32***, align 8
  %vasi8 = alloca i8***, align 8
  %vbool = alloca i1***, align 8
  %vuchar = alloca i8***, align 8
  %vchar = alloca i8***, align 8
  %vvoid = alloca void***, align 8
  %vusize = alloca i64***, align 8
  %visize = alloca i64***, align 8
  %vf64 = alloca double***, align 8
  %vf32 = alloca float***, align 8
  %vu64 = alloca i64***, align 8
  %vu32 = alloca i32***, align 8
  %vu8 = alloca i8***, align 8
  %vi64 = alloca i64***, align 8
  %vi32 = alloca i32***, align 8
  %vi8 = alloca i8***, align 8
  store volatile i8*** null, i8**** %vi8, align 8
  store volatile i32*** null, i32**** %vi32, align 8
  store volatile i64*** null, i64**** %vi64, align 8
  store volatile i8*** null, i8**** %vu8, align 8
  store volatile i32*** null, i32**** %vu32, align 8
  store volatile i64*** null, i64**** %vu64, align 8
  store volatile float*** null, float**** %vf32, align 8
  store volatile double*** null, double**** %vf64, align 8
  store volatile i64*** null, i64**** %visize, align 8
  store volatile i64*** null, i64**** %vusize, align 8
  store volatile void*** null, void**** %vvoid, align 8
  store volatile i8*** null, i8**** %vchar, align 8
  store volatile i8*** null, i8**** %vuchar, align 8
  store volatile i1*** null, i1**** %vbool, align 8
  store volatile i8*** null, i8**** %vasi8, align 8
  store volatile i32*** null, i32**** %vasi32, align 8
  store volatile i64*** null, i64**** %vasi64, align 8
  store volatile i8*** null, i8**** %vasu8, align 8
  store volatile i32*** null, i32**** %vasu32, align 8
  store volatile i64*** null, i64**** %vasu64, align 8
  store volatile float*** null, float**** %vasf32, align 8
  store volatile double*** null, double**** %vasf64, align 8
  store volatile i64*** null, i64**** %vasisize, align 8
  store volatile i64*** null, i64**** %vasusize, align 8
  store volatile void*** null, void**** %vasvoid, align 8
  store volatile i8*** null, i8**** %vaschar, align 8
  store volatile i8*** null, i8**** %vasuchar, align 8
  store volatile i1*** null, i1**** %vasbool, align 8
  br label %ret

ret:                                              ; preds = %entry
  ret void
}
