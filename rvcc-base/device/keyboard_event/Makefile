MAKEFLAGS = -sR
CC	:= gcc
LD	:= ld
CCFLAGES	:= -c -O0 -Wall
LKFLAGES :=
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
all: clean build link
build:$(ALL_OBJS)

run:
	./$(MAINEXEC)
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
	$(CC) $(LKFLAGES) -o $@ $(ALL_OBJS)
	@echo 'CC -[M] 正在构建... '$@