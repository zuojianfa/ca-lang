; ModuleID = 'primitives3_notzero.ca'
source_filename = "primitives3_notzero.ca"

declare i32 @printf(i8*, ...)

define void @main() {
entry:
  %vi8 = alloca i8***, align 8
  store volatile i8*** inttoptr (i32 1234567 to i8***), i8**** %vi8, align 8
  %vi32 = alloca i32***, align 8
  store volatile i32*** inttoptr (i32 1234567 to i32***), i32**** %vi32, align 8
  %vi64 = alloca i64***, align 8
  store volatile i64*** inttoptr (i32 1234567 to i64***), i64**** %vi64, align 8
  %vu8 = alloca i8***, align 8
  store volatile i8*** inttoptr (i32 1234567 to i8***), i8**** %vu8, align 8
  %vu32 = alloca i32***, align 8
  store volatile i32*** inttoptr (i32 1234567 to i32***), i32**** %vu32, align 8
  %vu64 = alloca i64***, align 8
  store volatile i64*** inttoptr (i32 1234567 to i64***), i64**** %vu64, align 8
  %vf32 = alloca float***, align 8
  store volatile float*** inttoptr (i32 1234567 to float***), float**** %vf32, align 8
  %vf64 = alloca double***, align 8
  store volatile double*** inttoptr (i32 1234567 to double***), double**** %vf64, align 8
  %visize = alloca i64***, align 8
  store volatile i64*** inttoptr (i32 1234567 to i64***), i64**** %visize, align 8
  %vusize = alloca i64***, align 8
  store volatile i64*** inttoptr (i32 1234567 to i64***), i64**** %vusize, align 8
  %vvoid = alloca void***, align 8
  store volatile void*** inttoptr (i32 1234567 to void***), void**** %vvoid, align 8
  %vchar = alloca i8***, align 8
  store volatile i8*** inttoptr (i32 1234567 to i8***), i8**** %vchar, align 8
  %vuchar = alloca i8***, align 8
  store volatile i8*** inttoptr (i32 1234567 to i8***), i8**** %vuchar, align 8
  %vbool = alloca i1***, align 8
  store volatile i1*** inttoptr (i32 1234567 to i1***), i1**** %vbool, align 8
  %vasi8 = alloca i8***, align 8
  store volatile i8*** inttoptr (i32 1234567 to i8***), i8**** %vasi8, align 8
  %vasi32 = alloca i32***, align 8
  store volatile i32*** inttoptr (i32 1234567 to i32***), i32**** %vasi32, align 8
  %vasi64 = alloca i64***, align 8
  store volatile i64*** inttoptr (i32 1234567 to i64***), i64**** %vasi64, align 8
  %vasu8 = alloca i8***, align 8
  store volatile i8*** inttoptr (i32 1234567 to i8***), i8**** %vasu8, align 8
  %vasu32 = alloca i32***, align 8
  store volatile i32*** inttoptr (i32 1234567 to i32***), i32**** %vasu32, align 8
  %vasu64 = alloca i64***, align 8
  store volatile i64*** inttoptr (i32 1234567 to i64***), i64**** %vasu64, align 8
  %vasf32 = alloca float***, align 8
  store volatile float*** inttoptr (i32 1234567 to float***), float**** %vasf32, align 8
  %vasf64 = alloca double***, align 8
  store volatile double*** inttoptr (i32 1234567 to double***), double**** %vasf64, align 8
  %vasisize = alloca i64***, align 8
  store volatile i64*** inttoptr (i32 1234567 to i64***), i64**** %vasisize, align 8
  %vasusize = alloca i64***, align 8
  store volatile i64*** inttoptr (i32 1234567 to i64***), i64**** %vasusize, align 8
  %vasvoid = alloca void***, align 8
  store volatile void*** inttoptr (i32 1234567 to void***), void**** %vasvoid, align 8
  %vaschar = alloca i8***, align 8
  store volatile i8*** inttoptr (i32 1234567 to i8***), i8**** %vaschar, align 8
  %vasuchar = alloca i8***, align 8
  store volatile i8*** inttoptr (i32 1234567 to i8***), i8**** %vasuchar, align 8
  %vasbool = alloca i1***, align 8
  store volatile i1*** inttoptr (i32 1234567 to i1***), i1**** %vasbool, align 8
  br label %ret

ret:                                              ; preds = %entry
  ret void
}
