
#include <elf.h>
#include <stdio.h>

void print_elf_type(uint16_t type_enum) {
  switch (type_enum) {
  case ET_REL:
    printf("A relocatable file.");
    break;
  case ET_DYN:
    printf("A shared object file.");
    break;
  case ET_NONE:
    printf("An unknown type.");
    break;
  case ET_EXEC:
    printf("An executable file.");
    break;
  case ET_CORE:
    printf("A core file.");
    break;
  }
}

int main(void) {
  Elf64_Ehdr elf_header;
  FILE *fp = fopen("./elf", "r");
  fread(&elf_header, sizeof(Elf64_Ehdr), 1, fp);
  print_elf_type(elf_header.e_type); // "An executable file."
  fclose(fp);
  return 0;
}
