MAKEFLAGS = -sR
CC	:= riscv64-unknown-elf-gcc
LD	:= riscv64-unknown-elf-ld
OBJCOPY	:= riscv64-unknown-elf-objcopy
OCFLAGES	:= -S -O binary
CCFLAGES	:= -c -march=rv32i -mabi=ilp32 -O3 -Wall -save-temps
LKFLAGES	:= -nostdlib -T ld.lds -Map main.Map
QEMU	:= qemu-riscv32
QFLAGES := -g 1234
RM := rm 
RMFLAGES := -rf 

ALLC_FILE := $(wildcard *.c)
ALLS_FILE := $(wildcard *.S)
ALLC_OBJS = $(ALLC_FILE:%.c=%.o)
ALLS_OBJS = $(ALLS_FILE:%.S=%.o)
ALL_OBJS  = $(ALLS_OBJS) $(ALLC_OBJS)
ALL_BINS  = $(ALL_OBJS:%.o=%.bin)
MAINEXEC = main.elf
.PHYANY: all clean build run
all: clean build link binary
build:$(ALL_OBJS)
binary:$(ALL_BINS)
run:
	$(QEMU) $(QFLAGES) $(MAINEXEC)
link:$(MAINEXEC)
clean:
	$(RM) $(RMFLAGES) *.elf *.o *.i *.s *.Map *.bin

CCSTR		= 	'CC -[M] 正在构建... '$<
PRINTCSTR 	=	@echo $(CCSTR)
%.o : %.c
	$(CC) $(CCFLAGES) -o $@ $<
	$(PRINTCSTR)
%.o : %.S
	$(CC) $(CCFLAGES) -o $@ $<
	$(PRINTCSTR)
%.bin : %.o
	$(OBJCOPY) $(OCFLAGES) $< $@
	$(PRINTCSTR)	
%.elf :
	$(LD) $(LKFLAGES) -o $@ $(ALL_OBJS)
	@echo 'CC -[M] 正在构建... '$@