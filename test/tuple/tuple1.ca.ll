; ModuleID = 'tuple1.ca'
source_filename = "tuple1.ca"

@0 = private unnamed_addr constant [22 x i8] c"size = %lu, type: %s\0A\00", align 1
@1 = private unnamed_addr constant [7 x i8] c"t:(AA)\00", align 1
@2 = private unnamed_addr constant [22 x i8] c"size = %lu, type: %s\0A\00", align 1
@3 = private unnamed_addr constant [11 x i8] c"t:(A1;i32)\00", align 1
@4 = private unnamed_addr constant [22 x i8] c"size = %lu, type: %s\0A\00", align 1
@5 = private unnamed_addr constant [12 x i8] c"t:(A2;bool)\00", align 1
@6 = private unnamed_addr constant [22 x i8] c"size = %lu, type: %s\0A\00", align 1
@7 = private unnamed_addr constant [11 x i8] c"t:(A3;f64)\00", align 1
@8 = private unnamed_addr constant [22 x i8] c"size = %lu, type: %s\0A\00", align 1
@9 = private unnamed_addr constant [16 x i8] c"t:(A4;f64,bool)\00", align 1
@10 = private unnamed_addr constant [22 x i8] c"size = %lu, type: %s\0A\00", align 1
@11 = private unnamed_addr constant [16 x i8] c"t:(A5;bool,f64)\00", align 1
@12 = private unnamed_addr constant [22 x i8] c"size = %lu, type: %s\0A\00", align 1
@13 = private unnamed_addr constant [20 x i8] c"t:(A6;bool,f64,i32)\00", align 1
@14 = private unnamed_addr constant [22 x i8] c"size = %lu, type: %s\0A\00", align 1
@15 = private unnamed_addr constant [30 x i8] c"t:{A6a;f1:bool,f2:f64,f3:i32}\00", align 1
@16 = private unnamed_addr constant [22 x i8] c"size = %lu, type: %s\0A\00", align 1
@17 = private unnamed_addr constant [30 x i8] c"t:(A7;i32,(A1;i32),(A2;bool))\00", align 1
@18 = private unnamed_addr constant [22 x i8] c"size = %lu, type: %s\0A\00", align 1
@19 = private unnamed_addr constant [85 x i8] c"t:(A8;i32,(A1;i32),(A2;bool),(A3;f64),(A4;f64,bool),(A5;bool,f64),(A6;bool,f64,i32))\00", align 1
@20 = private unnamed_addr constant [22 x i8] c"size = %lu, type: %s\0A\00", align 1
@21 = private unnamed_addr constant [118 x i8] c"t:(A9;(A7;i32,(A1;i32),(A2;bool)),(A8;i32,(A1;i32),(A2;bool),(A3;f64),(A4;f64,bool),(A5;bool,f64),(A6;bool,f64,i32)))\00", align 1

declare i32 @printf(i8*, ...)

define void @main() {
entry:
  %n = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @0, i32 0, i32 0), i64 0, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @1, i32 0, i32 0))
  %n1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @2, i32 0, i32 0), i64 4, i8* getelementptr inbounds ([11 x i8], [11 x i8]* @3, i32 0, i32 0))
  %n2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @4, i32 0, i32 0), i64 1, i8* getelementptr inbounds ([12 x i8], [12 x i8]* @5, i32 0, i32 0))
  %n3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @6, i32 0, i32 0), i64 8, i8* getelementptr inbounds ([11 x i8], [11 x i8]* @7, i32 0, i32 0))
  %n4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @8, i32 0, i32 0), i64 16, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @9, i32 0, i32 0))
  %n5 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @10, i32 0, i32 0), i64 16, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @11, i32 0, i32 0))
  %n6 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @12, i32 0, i32 0), i64 24, i8* getelementptr inbounds ([20 x i8], [20 x i8]* @13, i32 0, i32 0))
  %n7 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @14, i32 0, i32 0), i64 24, i8* getelementptr inbounds ([30 x i8], [30 x i8]* @15, i32 0, i32 0))
  %n8 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @16, i32 0, i32 0), i64 12, i8* getelementptr inbounds ([30 x i8], [30 x i8]* @17, i32 0, i32 0))
  %n9 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @18, i32 0, i32 0), i64 80, i8* getelementptr inbounds ([85 x i8], [85 x i8]* @19, i32 0, i32 0))
  %n10 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @20, i32 0, i32 0), i64 96, i8* getelementptr inbounds ([118 x i8], [118 x i8]* @21, i32 0, i32 0))
  br label %ret

ret:                                              ; preds = %entry
  ret void
}
