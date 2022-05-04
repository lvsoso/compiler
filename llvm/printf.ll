; file = printf.ll

@str = constant [18 x i8] c"%ld - 0x%x - %lf\0A\00"

declare i32 @printf(i8*, ...)

define i32 @main(){
  
  %t0 = add i32 10, 5

  %fmt = getelementptr [18 x i8], [18 x i8]* @str, i32 0 , i32 0

  call i32 (i8*, ...) @printf(i8* %fmt, i64 -10, i32 %t0, double 0.125)

  ret i32 0
}
