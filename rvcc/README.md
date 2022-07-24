
## rvcc

学习相关代码 https://github.com/sunshaoce/rvcc

### 构建

#### Makefile

```shell
make
```

#### CMake

```shell
cmake -Bbuild .
cd build/
make
```



### 环境准备

#### 相关

[chibicc](https://github.com/rui314/chibicc)
[rvcc](https://github.com/sunshaoce/rvcc)
[plct mirror](https://mirror.iscas.ac.cn/plct/)

#### 依赖

```shell
sudo apt-get install autoconf automake autotools-dev curl libmpc-dev libmpfr-dev libgmp-dev libusb-1.0.0-dev gawk build-essential bison flex texinfo gperf libtool patchutils bc zlib1g-dev device-tree-compiler pkg-config libexpat-dev cmake libglib2.0-dev libpixman-1-dev
```

#### 安装步骤

```shell
# 设置环境变量
RISCV=$HOME/RISCV/bin/riscv64/
RISCV_BIN=$RISCV/bin/
RISCV_ELF=$RISCV/riscv64-unknown-elf/
LLVM_BIN=$HOME/RISCV/bin/llvm/bin/
QEMU_BIN=$HOME/RISCV/bin/qemu/bin/
PATH=$PATH:$RISCV:$RISCV_BIN:$RISCV_ELF:$LLVM_BIN:$QEMU_BIN

# 创建目录
mkdir $HOME/RISCV/


# 依次克隆代码代码仓库并编译安装
CURRENT=$PWD

git clone https://github.com/riscv/riscv-gnu-toolchain --recursive
cd riscv-gnu-toolchain && mkdir build && cd build
#../configure --prefix=$HOME/RISCV/toolchain/riscv32 --with-arch=rv32gc --with-abi=ilp32d
#make
../configure --prefix=$RISCV  --with-arch=rv64gc --with-abi=lp64d
make

cd $CURRENT

git clone https://github.com/riscv-software-src/riscv-pk.git --recursive
cd riscv-pk && mkdir build && cd build
../configure --prefix=$RISCV  --host=riscv64-unknown-elf
make
make install

cd $CURRENT

git clone https://github.com/riscv-software-src/riscv-isa-sim.git --recursive
cd riscv-isa-sim && mkdir build && cd build
../configure --prefix=$RISCV
make
make install

cd $CURRENT

git clone http://github.com/isrc-cas/rvv-llvm.git --recursive
cd rvv-llvm && mkdir build && cd build
cmake -DLLVM_TARGETS_TO_BUILD="RISCV" -DLLVM_ENABLE_PROJECTS=clang -DCMAKE_INSTALL_PREFIX=$HOME/RISCV/bin/llvm -G "Unix Makefiles" ../llvm
make
make install

cd $CURRENT

git clone https://git.qemu.org/git/qemu.git --recursive
cd qemu && mkdir build && cd build
../configure --target-list=riscv64-linux-user,riscv64-softmmu --prefix=$HOME/RISCV/bin/qemu
make
make install
```

#### 编写代码测试

hello.c

```C
#include <stdio.h>

int main()
{
    printf("hello world!\n");
    return 0;
}
```

#### 编译运行

需要设置前面步骤的环境变量。

```shell
riscv64-unknown-elf-gcc hello.c -o hello
spike pk hello

# clang --target=riscv64-unknown-elf -march=rv64gv hello.c --sysroot=$RISCV_ELF --gcc-toolchain=$RISCV -o hello
clang-12 --target=riscv64-unknown-elf -march=rv64gc hello.c --sysroot=$RISCV_ELF --gcc-toolchain=$RISCV -o hello
qemu-risc64 hello
```
