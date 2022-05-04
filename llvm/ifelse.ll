; file = ifelse.ll

@str0 = constant [11 x i8] c"positive!\0A\00"
@str1 = constant [11 x i8] c"negative!\0A\00"


declare i32 @printf(i8*, ...)

define i32 @main(){
_ifstart:
   %a = add i32 1, 0
   %cond = icmp sgt i32 %a, 0 ; sgt >

   ; if
   br i1 %cond, label %_iftrue, label %_iffalse

   _iftrue:
     %tmp0 = getelementptr [11 x i8], [11 x i8]* @str0, i32 0, i32 0
     call i32 (i8*, ...) @printf(i8* %tmp0)
     br label %_ifend

   _iffalse:
     %tmp1 = getelementptr [11 x i8], [11 x i8]* @str1, i32 0, i32 0
     call i32 (i8*, ...) @printf(i8* %tmp1)
     br label %_ifend

   _ifend:
     ret i32 0
}
