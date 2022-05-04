; file = calc.ll

@str = constant [4 x i8] c"%d\0A\00"

declare i32 @printf(i8*, ...)

; calculate (10 x 11 -1) / (2 + 6)
define i32 @main(){
  %tmp0 = add i32 2, 6
  %tmp1 = mul i32 10, 11
  %tmp2 = sub i32 %tmp1, 1
  %tmp3 = udiv i32 %tmp2, %tmp0

  %tmp4 = getelementptr [4 x i8], [4 x i8]* @str, i32 0, i32 0
  call i32 (i8*, ...) @printf(i8* %tmp4, i32 %tmp3)

  ret i32 0
}

