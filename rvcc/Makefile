CFLAGS=-std=c11 -g -fno-common

CC=clang
# CC=/home/lv/RISCV/bin/llvm/bin/clang-12

# C source code files, all endswith .c files.
SRCS=$(wildcard *.c)
# object file gen by C source code files compiled
# objs have the same file name with source file.
OBJS=$(SRCS:.c=.o)

#  $@ mean target file
# $^ mean dependence file
rvcc: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJS): rvcc.h

TEST_SRCS=$(wildcard test/*.c)
TESTS=$(TEST_SRCS:.c=.elf)
RISCV=$(HOME)/RISCV/bin/riscv64/

# 测试标签，运行测试
test/%.elf: rvcc test/%.c
	test-build-elf.sh $*

test: $(TESTS)
#	for i in $^; do echo $$i; ./$$i || exit 1; echo; done
#	for i in $^; do echo $$i; $(RISCV)/bin/qemu-riscv64 -L $(RISCV)/sysroot ./$$i || exit 1; echo; done
	for i in $^; do echo $$i; test-run-elf.sh ./$$i; echo; done
	test/driver.sh

# 清理标签，清理所有非源代码文件
clean:
	rm -rf rvcc tmp* $(TESTS) test/*.s test/*.exe
	find * -type f '(' -name '*~' -o -name '*.o' ')' -exec rm {} ';'

.PHONY: test clean