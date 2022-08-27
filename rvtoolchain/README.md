## 启动

```shell
PATH=/opt/riscv/gcc/bin/:/opt/riscv/qemu/bin/:$PATH code .
```

## RISC-V 工具链

#### 安装依赖工具：在宿主平台上安装编译器 A，以及相应的工具和库。
```shell
sudo apt-get install git autoconf automake autotools-dev curl python3 libmpc-dev libmpfr-dev libgmp-dev gawk build-essential bison flex texinfo gperf patchutils bc libexpat-dev libglib2.0-dev ninja-build zlib1g-dev pkg-config libboost-all-dev libtool libssl-dev libpixman-1-dev libpython-dev virtualenv libmount-dev libsdl2-dev
```

#### 下载 RISC-V 工具链的源代码；
- riscv-qemu（虚拟机）
- riscv-newlib (用嵌入式的轻量级C库)
- riscv-binutils(包含一些二进制工具集合，如objcopy等)
- riscv-gdb(用于调试代码的调试器)
- riscv-dejagnu(用于测试其它程序的框架)
- riscv-glibc(GNU的C库)
- riscv-gcc (C语言编译器)

```shell
git clone https://gitee.com/mirrors/riscv-gnu-toolchain
cd riscv-gnu-toolchain

# RISC-V 平台的 C 语言编译器源代码仓库
git clone -b riscv-gcc-10.2.0 https://gitee.com/mirrors/riscv-gcc

# 测试框架源代码仓库
git clone https://gitee.com/mirrors/riscv-dejagnu

# GNU 的 C 库源代码仓库
git clone -b riscv-glibc-2.29 https://gitee.com/mirrors/riscv-glibc

# 用于嵌入式的轻量级 C 库源代码仓库
git clone https://gitee.com/mirrors/riscv-newlib

# 二进制工具集合源代码仓库 riscv-binutils
git clone -b riscv-binutils-2.35 https://gitee.com/mirrors/riscv-binutils-gdb riscv-binutils

# GDB 软件调试器源代码仓库 riscv-gdb
git clone -b fsf-gdb-10.1-with-sim https://gitee.com/mirrors/riscv-binutils-gdb riscv-gdb
```

#### 配置 RISC-V 工具链；
```shell
mkdir build  #建立build目录

#配置操作，终端一定要切换到build目录下再执行如下指令
# –prefix 表示 RISC-V 的工具链的安装目录
# –enable-multilib 表示使用 multlib 库，使用该库编译出的 RISC-V 工具链，既可以生成 RISCV32 的可执行程序，也可以生成 RISCV64 的可执行程序，而默认的 Newlib 库则不行，它只能生成 RISCV（32/64）其中之一的可执行程序。
# –target 表示生成的 RISC-V 工具链中，软件名称的前缀是 riscv64-multlib-elf-xxxx
../configure --prefix=/opt/riscv/gcc --enable-multilib --target=riscv64-multlib-elf
```

#### 编译 RISC-V 工具链，并安装在宿主平台上。
```shell
sudo make -j4
```

##### 检查
```shell
cd /opt/riscv/gc/bin
riscv64-unknown-elf-gcc -v
```

#####  问题处理
```shell
remote: Enumerating objects: 9817, done.
error: RPC failed; curl 56 GnuTLS recv error (-9): A TLS packet with unexpected length was received.
fatal: The remote end hung up unexpectedly
fatal: early EOF
fatal: index-pack failed
```
##### 处理
```shell
sudo apt-get install gnutls-bin
git config --global http.sslVerify false
git config --global http.postBuffer 1048576000
```

#### Qemu

```shell
wget https://download.qemu.org/qemu-6.2.0.tar.xz #下载源码包
tar xvJf qemu-6.2.0.tar.xz #解压源码包


mkdir build #建立build目录

cd build #切换到build目录下

# –prefix 表示 QEMU 的安装目录，约定为“/opt/riscv/qemu”目录
# –enable-sdl 表示 QEMU 使用 sdl 图形库
# --enable-tools 表示生成 QEMU 工具集
# –enable-debug 表示打开 QEMU 调试功能。
# riscv32-softmmu rv32 系统模式平台,riscv64-softmmu rv64 系统模式平台,riscv32-linux-user rv32 用户模式平台,riscv64-linux-user rv64 用户模式平台
../configure --prefix=/opt/riscv/qemu --enable-sdl --enable-tools --enable-debug --target-list=riscv32-softmmu,riscv64-softmmu,riscv32-linux-user,riscv64-linux-user #配置QEMU

sudo make -j4
sudo make install

cd /opt/riscv/qemu/bin
qemu-riscv32 -version && qemu-riscv64 -version && qemu-system-riscv32 -version && qemu-system-riscv64 -version
```

#### 环境变量
```shell

cd ~ #切换到当前用户目录下
vim ./.bashrc #打开.bashrc文件进行编辑

#在.bashrc文件末尾加入如下信息
export PATH=/opt/riscv/gcc/bin:$PATH
export PATH=/opt/riscv/qemu/bin:$PATH
```