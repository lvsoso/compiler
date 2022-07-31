CFLAGS=-std=c11 -g -fno-common

CC=clang
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

test: rvcc
	./test.sh

clean:
	rm -f rvcc *.o *.s tmp* a.out

.PHONY: test clean