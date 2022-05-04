source_filename = "limit-loop.ll"

@str = constant [4 x i8] c"%d\0A\00"

declare i32 @printf(i8*, ...)

define dso_local i32 @main(){

; init value
_l0:
  ; i = 0
  %t0 = add i32 0, 0
  br label %_l1


; handle condition
_l1:
  ; i < 10
  %t1 = phi i32 [%t0, %_l0], [%t3, %_l2]
  %c = icmp slt i32 %t1, 10 ; slt <
  br i1 %c, label %_l2, label %_l3

_l2:
  %fmt = getelementptr [4 x i8], [4  x i8]* @str, i32 0, i32 0 
  %t2 = call i32 (i8*, ...) @printf(i8* %fmt, i32 %t1)

  ; i++
  %t3 = add i32 %t1, 1
  br label %_l1

_l3:
  ret i32 0
}
