
source_filename = "phi.ll"

@str = constant [4 x i8] c"%d\0A\00"

declare i32 @printf(i8*, ...)


define dso_local i32 @main() {

  _ifstart:
    %a = add i32 10, 0
    %c = icmp sgt i32 %a, 0
    br i1 %c, label %_iftrue, label %_iffalse

  _iftrue:
    %t0 = add i32 %a, 1
    br label %_ifend

  _iffalse:
    %t1 = add i32 %a, -1
    br label %_ifend

  _ifend:
    %t2 = phi i32 [%t0, %_iftrue], [%t1, %_iffalse]
    %t3 = sdiv i32 %t2, 2
  
    %fmt = getelementptr [4 x i8], [4 x i8]* @str, i32 0, i32 0

    %t4 = call i32 (i8*, ...) @printf(i8* %fmt, i32 %t3)
    ret i32 0
}
