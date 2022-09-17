void sw_ins(unsigned int addr, int val);

unsigned int word = 0xffffffff;

int main() {
  sw_ins((unsigned int)&word, 0);
  return 0;
}
