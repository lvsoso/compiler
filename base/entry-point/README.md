# 程序运行时入口
## _start
```shell
gcc main.c -o main
readelf -h ./main
objdump -M intel -d ./main | grep 4f0 -A 10

(base) lv@lv:entry-point$ readelf -h ./main
ELF 头：
  Magic：   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00 
  类别:                              ELF64
  数据:                              2 补码，小端序 (little endian)
  版本:                              1 (current)
  OS/ABI:                            UNIX - System V
  ABI 版本:                          0
  类型:                              DYN (共享目标文件)
  系统架构:                          Advanced Micro Devices X86-64
  版本:                              0x1
  入口点地址：               0x4f0 ### <-----
  程序头起点：          64 (bytes into file)
  Start of section headers:          6376 (bytes into file)
  标志：             0x0
  本头的大小：       64 (字节)
  程序头大小：       56 (字节)
  Number of program headers:         9
  节头大小：         64 (字节)
  节头数量：         28
  字符串表索引节头： 27

(base) lv@lv:entry-point$ objdump -M intel -d ./main | grep 4f0 -A 10
00000000000004f0 <_start>:
 4f0:   31 ed                   xor    ebp,ebp
 4f2:   49 89 d1                mov    r9,rdx
 4f5:   5e                      pop    rsi
 4f6:   48 89 e2                mov    rdx,rsp
 4f9:   48 83 e4 f0             and    rsp,0xfffffffffffffff0
 4fd:   50                      push   rax
 4fe:   54                      push   rsp
 4ff:   4c 8d 05 7a 01 00 00    lea    r8,[rip+0x17a]        # 680 <__libc_csu_fini>
 506:   48 8d 0d 03 01 00 00    lea    rcx,[rip+0x103]        # 610 <__libc_csu_init>
 50d:   48 8d 3d e6 00 00 00    lea    rdi,[rip+0xe6]        # 5fa <main>
 514:   ff 15 c6 0a 20 00       call   QWORD PTR [rip+0x200ac6]        # 200fe0 <__libc_start_main@GLIBC_2.2.5>
```

### CRT

CRT 为应用程序提供了对启动与退出、C 标准库函数、IO、堆、C 语言特殊实现、调试等多方面功能的实现和支持。

CRT 的实现是平台相关的，它与具体操作系统结合得非常紧密。

- crt1.o 是由 C 运行时库（C Runtime Library，CRT）提供的一个用于辅助应用程序正常运行的特殊对象文件，该文件在其内部定义了符号 _start 对应的具体实现。

- crti.o 和 crtn.o，两者通过共同协作，为共享对象提供了可以使用“构造函数”与“析构函数”的能力；

- crtbegin.o 和 crtend.o，分别提供了上述“构造函数”与“析构函数”中的具体代码实现。

