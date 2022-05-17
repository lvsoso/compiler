; loop.ll


@str = constant [15 x i8] c"hello, world!\0A\00"

declare i32 @printf(i8*, ...)

define i32 @main(){
  br label %Dest

  Dest:
   %tmp0 = getelementptr [15 x i8], [15 x i8]* @str, i32 0, i32 0
   call i32 (i8*, ...) @printf(i8* %tmp0)
   br label %Dest
}
