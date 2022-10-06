
# 动态链接
- export 修改 LD_LIBRARY_PATH 变量;
- rpath: 将动态库所在路径嵌入到可执行文件;
- ldconfig:将共享库安装到当前系统可访问的全局环境中;

```shell
gcc sum.c -shared -fPIC -o libsum.so 
gcc -o main main.c -lsum -L.
LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH  ./main
```

### 位置无关代码
位置无关代码（Position Independent Code，PIC）是一类特殊的机器代码，这些代码在使用时，可以被放置在每个进程 VAS 中的任意位置，而无需链接器对它内部引用的地址进行重定位。

### 全局偏移表
全局偏移表（Global Offset Table，GOT）是位于每个模块 Data Segment 起始位置处的一个特殊表结构，其内部的每个表项中都存放有一个地址信息。而这些地址便分别对应于被当前模块引用的外部函数或变量在进程 VAS 中的实际地址。

当程序被加载进内存时，动态链接器就可以根据实际情况，通过修正 GOT 表项中的值，来做到间接修正代码中对应符号的实际引用地址。

### 过程链接表（Procedure Linkage Table，PLT）

虽然我们可以让动态链接器在程序加载时，将其代码中使用到的所有外部符号地址，更新在相应的 GOT 表项中，但当程序依赖的外部符号越来越多时，重定位的成本也会越来越高。

会导致程序初次运行时的“启动延迟”逐渐变大，甚至影响到程序正常功能的运作。

- PLT[0]（即 PLT 中的第一个表项）存放的代码专门用于调用动态链接器；
- 其他表项中则依次存放用于完成用户函数调用过程的相关代码，这些表项的地址将被程序中的 call 指令直接使用。

在 ELF 文件中，GOT 对应 Section 的 .got 与 .got.plt 两个部分。其中，前者主要用于保存相关全局变量的地址信息；而后者则主要参与到函数符号的延迟绑定过程中。

#### .got.plt 中的前三个表项
- 第一个表项中保存的是 .dynamic 的地址。这个 Section 中保存了动态链接器需要使用的一些信息；
- 第二个表项中保存的是当前模块的描述符 ID；
- 第三个表项中保存的是函数 _dl_runtime_resolve 的地址。该函数由操作系统的运行时环境提供，它将参与到 GOT 的运行时重定位过程中。


## 加载时链接

基于 GOT 与 PLT 两个表结构进行。

主要特征是，动态链接器进行的符号重定位过程发生在程序代码被真正执行之前。

在执行程序代码前，内核会首先根据名为 .interp 的 Section 中的内容，将相应的动态链接器共享库（ld.so）映射至进程的 VAS 中，并同时将控制权转移给它。

动态链接器在执行过程中，会通过其自身 .dynamic 中记录的信息，来完成对自己的重定位工作。

接着，通过访问应用程序的 .dynamic，动态链接器可以获得它依赖的所有外部共享库，并在此基础之上完成对整个程序的动态链接过程。

## 运行时链接

符号的重定位发生在程序的运行过程中。

这种方式有时也被称为“动态载入”或“运行时加载”，它的基本原理与正常的动态链接完全一致，只是链接的发生过程被推迟到了程序运行时。

通过这种方式，程序可以自由选择想要加载的共享库模块，并在不使用时及时卸载，程序的模块化组织变得更加灵活。

四个 API
- dlopen
- dlsym
- dlerror
-  dlclose 

```shell
cd load-in-runtime
gcc -o main main.c  -lm -ldl
```