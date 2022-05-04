; file = cmp.ll

@str = constant [9 x i8] c"%d - %d\0A\00"

declare i32 @printf(i8*, ...)

define i32 @main() {
  %tmp0 = add i32 2, 6
  %tmp1 = mul i32 10, 11
  %tmp2 = icmp eq i32 %tmp0, %tmp1 ; eq ==
  %tmp3 = icmp ult i32 %tmp0, %tmp1 ; ult <

  %tmp4 = getelementptr [9 x i8], [9 x i8]* @str, i32 0, i32 0
  call i32 (i8*, ...) @printf(i8* %tmp4, i1 %tmp2, i1 %tmp3)

  ret i32 0
}
