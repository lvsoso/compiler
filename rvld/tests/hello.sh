#! /bin/bash

test_name=$(basename "$0" .sh)
t=out/tests/$test_name

mkdir -p "$t"

cat <<EOF | riscv64-linux-gnu-gcc -o "$t"/a.out -c -xc -
#include <stdio.h>

int main(void){
    printf("Hello, World\n");
    return 0;
}
EOF

echo "$t"/a.out