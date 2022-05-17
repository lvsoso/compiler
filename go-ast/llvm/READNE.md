### hello
#### 编写hello.ll
```shell
; hello.ll

; constant define constant value
; [15 x i8] a contant array has 15 elements which type i8
@str = constant [15 x i8] c"hello, world!\0A\00"

; declare: declare the function in other .ll file or lib 
; i32: return vaule's type
; printf: c's printf function
; i8*: 8 bit pointer
;  and other input data
declare i32 @printf(i8*, ...)


define i32 @main() {

  ; getelementptr [M x i8], [M x i8] * &str, i32 0, i32 0
  ; M: number of elements
  ; return i8*
  %tmpl = getelementptr [15 x i8], [ 15 x i8] * @str, i32 0, i32 0


  ; call: LLVM IR instruction
  ; i32 (i8*, ...): return value ordered list
  call i32 (i8*, ...) @printf(i8* %tmpl)

  ret i32 0
}
```

#### 编译执行

```shell
#  将 LLVM-IR 编译成 x86-64 平台的汇编指令
(base) lv@lv:llvm$ llc hello.ll -o hello.s

# 将 x86 平台的汇编指令汇编链接成最终的可执行文件
(base) lv@lv:llvm$ clang hello.s  -o hello.out

(base) lv@lv:llvm$ ./hello.out 
hello, world!

(base) lv@lv:llvm$ file hello.out 
hello.out: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, for GNU/Linux 3.2.0, not stripped

```

### printf.ll
```shell
echo """
; file = printf.ll

@str = constant [18 x i8] c"%ld - 0x%x - %lf\0A\00"

declare i32 @printf(i8*, ...)

define i32 @main(){
  
  %t0 = add i32 10, 5

  %fmt = getelementptr [18 x i8], [18 x i8]* @str, i32 0 , i32 0

  call i32 (i8*, ...) @printf(i8* %fmt, i64 -10, i32 %t0, double 0.125)

  ret i32 0
}

""" > printf.ll



(base) lv@lv:llvm$ llc  printf.ll -o printf.s
(base) lv@lv:llvm$ clang printf.s  -o printf.out
(base) lv@lv:llvm$ ./printf.out 
-10 - 0xf - 0.125000
```

### 四则运算

```shell
echo """
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

""" > calc.ll

(base) lv@lv:llvm$ llc calc.ll  -o calc.s
(base) lv@lv:llvm$ clang calc.s -o calc.out
(base) lv@lv:llvm$ ./calc.out 
13
```

### 比较运算

```shell
echo """
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

""" > cmp.ll

(base) lv@lv:llvm$ llc cmp.ll 
(base) lv@lv:llvm$ clang cmp.s -o cmp.out
(base) lv@lv:llvm$ ./cmp.out 
0 - 1

```

### 分支循环
```shell
echo """
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

""" > ifele.ll


(base) lv@lv:llvm$ llc ifelse.ll -o ifelse.s
(base) lv@lv:llvm$ clang ifelse.s -o ifelse.out
(base) lv@lv:llvm$ ./ifelse.out 
positive!
```
#### 基本块
- 指令组成基本块，基本块组成函数，函数组成可执行程序；
- 每个基本块的最后一条指令一定是跳转指令或返回指令，开头的指令和中间的指令一定不能是跳转指令；
- 基本块之间有跳转关系，跳出的基本块称为前驱，跳入的基本块称为后继；

ifelse.ll 的main函数包含4个基本块，分别是_ifstart, _iftrue, _iffalse, _ifend。

### 循环
```shell
echo """
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

"""

(base) lv@lv:llvm$ llc loop.ll 
(base) lv@lv:llvm$ clang loop.s -o loop.out
(base) lv@lv:llvm$ ./loop.out 
```

### PHI 指令
临时变量tmpx的值再运行时动态决定，可能是来之基本块block0的临时变量tmp0,也可能是来自基本块block1的临时变量tmp1.
> %tmpx = phi type [%tmp0, %block0], [%tmp1, %block1]

```shell
echo """
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
"""

(base) lv@lv:llvm$ llc phi.ll 
(base) lv@lv:llvm$ clang phi.s phi.out
(base) lv@lv:llvm$ clang phi.s -o phi.out
(base) lv@lv:llvm$ ./phi.out 
5
```

### 有限循环
- 初始化： 循环变量赋值
- 循环条件：判断是继续循环还是退出循环
- 循环体：执行业务逻辑，并更新循环变量

```shell
echo """
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

""" > limit-loop.ll

(base) lv@lv:llvm$ llc limit-loop.ll 
(base) lv@lv:llvm$ clang limit-loop.s -o limit-loop.out
(base) lv@lv:llvm$ ./limit-loop.out 
0
1
2
3
4
5
6
7
8
9
```

