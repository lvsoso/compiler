
# relation section

- .symtab 符号表，存放源代码中定义和引用的所有函数与全局变量的信息
- .rela.text 重定位信息表。链接器将当前文件与其他文件通过静态链接组合时，.text中需要被修改的代码
- .rela.data 重定位信息表。链接器将当前文件与其他文件通过静态链接组合时，.data中需要被修改的值



```shell
gcc main.c sum.c -o main  # 生成可执行文件 main；
gcc -c main.c -o main.o  # 生成目标文件 main.o；
gcc -c sum.c -o sum.o  # 生成目标文件 sum.o；
```

# nm
- D 已初始化的全局数据，位于 .data Section
- T 函数，位于 .text Section
- U 未定义
- 
```shell
(base) lv@lv:static$ nm main.o 
0000000000000000 D array
                 U _GLOBAL_OFFSET_TABLE_
0000000000000000 T main
                 U sharedArr
                 U sum
(base) lv@lv:static$ nm sum.o
0000000000000000 D sharedArr
```

# 静态链接
在链接过程中，来自于不同目标文件的代码会被整合为二进制可执行文件的一部分

> 在 GCC 中，我们可以通过为函数显式添加 __attribute__((weak)) 标记的形式，来将它标记为弱符号,会对代码的可移植性产生一定影响。

## 符号解析
- 如果有一个强符号和多个弱符号同名，则选择强符号；
- 如果有多个弱符号同名，则从这些弱符号中任意选一个（通常会选择其中类型占用空间最大的那个）；
- 如果存在多个强符号同名，则抛出链接错误。

## 重定位
-  偏移量: Offset 属性，表明该重定向目标在对应 Section 中的偏移；
-  类型: Type 属性表明了重定位类型，即链接器在处理该重定位表项时，需要使用的特定方式；
-  符号值: Sym.Value 属性为当前重定位符号的值；
-  符号名称 + 加数: Sym. Name + Addend” 属性为符号的名称，外加在计算该符号地址时需要被修正的量。


#### X86-64 重定向类型
- R_X86_64_NONE ： none
- R_X86_64_64 ：S+A
- R_X86_64_PC32：S+A-P
- R_X86_64_GOT32：G+A
- R_X86_64_PLT32：L+A-P
- R_X86_64_COPY：none

```shell
(base) lv@lv:static$ readelf -r ./main.o 

重定位节 '.rela.text' at offset 0x260 contains 2 entries:
  偏移量          信息           类型           符号值        符号名称 + 加数
00000000000b  000900000002 R_X86_64_PC32     0000000000000000 array - 4
000000000018  000d00000004 R_X86_64_PLT32    0000000000000000 sum - 4

重定位节 '.rela.data.rel' at offset 0x290 contains 1 entry:
  偏移量          信息           类型           符号值        符号名称 + 加数
000000000000  000a00000001 R_X86_64_64       0000000000000000 sharedArr + 0

重定位节 '.rela.eh_frame' at offset 0x2a8 contains 1 entry:
  偏移量          信息           类型           符号值        符号名称 + 加数
000000000020  000200000002 R_X86_64_PC32     0000000000000000 .text + 0

(base) lv@lv:static$ readelf -r ./sum.o 

重定位节 '.rela.eh_frame' at offset 0x200 contains 1 entry:
  偏移量          信息           类型           符号值        符号名称 + 加数
000000000020  000200000002 R_X86_64_PC32     0000000000000000 .text + 0

(base) lv@lv:static$ objdump -M intel -d main.o

main.o：     文件格式 elf64-x86-64


Disassembly of section .text:

0000000000000000 <main>:
   0:   55                      push   rbp
   1:   48 89 e5                mov    rbp,rsp
   4:   48 83 ec 10             sub    rsp,0x10
   8:   48 8b 05 00 00 00 00    mov    rax,QWORD PTR [rip+0x0]        # f <main+0xf>
   f:   be 02 00 00 00          mov    esi,0x2
  14:   48 89 c7                mov    rdi,rax
  17:   e8 00 00 00 00          call   1c <main+0x1c>
  1c:   89 45 fc                mov    DWORD PTR [rbp-0x4],eax
  1f:   8b 45 fc                mov    eax,DWORD PTR [rbp-0x4]
  22:   c9                      leave  
  23:   c3                      ret    
(base) lv@lv:static$ objdump -s -j .data ./main.o

./main.o：     文件格式 elf64-x86-64

```

### 重定位修改位置计算

S + A - P

0x201010 + (- 0x4) - （0x622 + 3）= 0x2009e7

0x2009e7 -> 0x002009e7

```shell
# 查看main
(base) lv@lv:static$ readelf -r ./main.o 

重定位节 '.rela.text' at offset 0x260 contains 2 entries:
  偏移量          信息           类型           符号值        符号名称 + 加数
00000000000b  000900000002 R_X86_64_PC32     0000000000000000 array - 4
000000000018  000d00000004 R_X86_64_PLT32    0000000000000000 sum - 4


(base) lv@lv:static$ nm ./main | grep array
0000000000201010 D array
0000000000200df8 t __do_global_dtors_aux_fini_array_entry
0000000000200df0 t __frame_dummy_init_array_entry
0000000000200df8 t __init_array_end
0000000000200df0 t __init_array_start

(base) lv@lv:static$ objdump -M intel -d main

main：     文件格式 elf64-x86-64


Disassembly of section .init:

00000000000004d0 <_init>:
 4d0:   48 83 ec 08             sub    rsp,0x8
...
000000000000061a <main>:
 61a:   55                      push   rbp
 61b:   48 89 e5                mov    rbp,rsp
 61e:   48 83 ec 10             sub    rsp,0x10
 #  622:   48 8b 05
 622:   48 8b 05 e7 09 20 00    mov    rax,QWORD PTR [rip+0x2009e7]        # 201010 <array>
 629:   be 02 00 00 00          mov    esi,0x2
 62e:   48 89 c7                mov    rdi,rax
 631:   e8 08 00 00 00          call   63e <sum>
 636:   89 45 fc                mov    DWORD PTR [rbp-0x4],eax
 639:   8b 45 fc                mov    eax,DWORD PTR [rbp-0x4]
 63c:   c9                      leave  
 63d:   c3                      ret    

```