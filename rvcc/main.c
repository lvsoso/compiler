#include <stdio.h>
#include <stdlib.h>

int main(int Argc, char **Argv) {
    if (Argc != 2) {
        fprintf(stderr, "%s: invalid number of arguments\n", Argv[0]);
        return 1;
    }

    char *P = Argv[1];

    printf("  .globl main\n");
    printf("main:\n");

    // 传入第一个number
    // strtol(target, remainder， base)
    printf(" li a0, %ld\n", strtol(P, &P, 10));

    // 解析
    while (*P)
    {
        if(*P == '+'){
            ++P;

            // addi rd, rs1, imm 表示 rd = rs1 + imm
            printf(" addi a0, a0, %ld\n", strtol(P, &P, 10));
            continue;
        }

          if (*P == '-') {
            ++P;
            // addi中imm为有符号立即数，所以减法表示为 rd = rs1 + (-imm)
            printf("  addi a0, a0, -%ld\n", strtol(P, &P, 10));
            continue;
            }

            fprintf(stderr, "unexpected character: '%c'\n", *P);
            return 1;
    }

    // jalr x0, x1, 0
    printf("  ret\n");
    return 0;
}

