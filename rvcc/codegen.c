#include "rvcc.h"

// 记录栈深度
static int Depth;

// 压栈，将结果临时压入栈中备用
// sp为栈指针，栈反向向下增长，64位下，8个字节为一个单位，所以sp-8
// 当前栈指针的地址就是sp，将a0的值压入栈
// 不使用寄存器存储的原因是因为需要存储的值的数量是变化的。
static void push(void)
{
  // sp  = sp - 8
  printf("  addi sp, sp, -8\n");
  // pos[sp+0]= a0
  printf("  sd a0, 0(sp)\n");
  Depth++;
}

// 弹栈，将sp指向的地址的值，弹出到a1
static void pop(char *Reg)
{
  // reg = pos[sp+0]
  printf("  ld %s, 0(sp)\n", Reg);
  // sp = sp + 8
  printf("  addi sp, sp, 8\n");
  Depth--;
}

// align N times to the 'Align'
static int alignTo(int N, int Align)
{
  // (0,Align]
  return (N + Align - 1) / Align * Align;
}

// get variables address
static void genAddr(Node *Nd)
{
  if (Nd->Kind == ND_VAR)
  {
    // 'offset' is compare with 'fp'
    printf("  addi a0, fp, %d\n", Nd->Var->Offset);
    return;
  }

  error("not an lvalue");
}

// 生成表达式
static void genExpr(Node *Nd)
{
  switch (Nd->Kind)
  {
  // 加载数字到a0
  case ND_NUM:
    printf("  li a0, %d\n", Nd->Val);
    return;
  // 对寄存器取反
  case ND_NEG:
    genExpr(Nd->LHS);
    // neg a0, a0是 sub a0, x0的别名, a0，即a0=0-a0
    printf("  neg a0, a0\n");
    return;
  // 变量
  case ND_VAR:
    // 计算出变量的地址，然后存入a0
    genAddr(Nd);
    // 访问a0地址中存储的数据，存入到a0当中
    printf("  ld a0, 0(a0)\n");
    return;
  // 赋值
  case ND_ASSIGN:
    // 左部是左值，保存值到的地址
    genAddr(Nd->LHS);
    push();
    // 右部是右值，为表达式的值
    genExpr(Nd->RHS);
    pop("a1");
    printf("  sd a0, 0(a1)\n");
    return;
  default:
    break;
  }
  // 递归到最右节点
  genExpr(Nd->RHS);
  // 将结果压入栈
  push();
  // 递归到左节点
  genExpr(Nd->LHS);
  // 将结果弹栈到a1
  pop("a1");

  // 生成各个二叉树节点
  switch (Nd->Kind)
  {
  case ND_ADD: // + a0=a0+a1
    printf("  add a0, a0, a1\n");
    return;
  case ND_SUB: // - a0=a0-a1
    printf("  sub a0, a0, a1\n");
    return;
  case ND_MUL: // * a0=a0*a1
    printf("  mul a0, a0, a1\n");
    return;
  case ND_DIV: // / a0=a0/a1
    printf("  div a0, a0, a1\n");
    return;
  case ND_EQ:
  case ND_NE:
    // a0=a0^a1，异或指令
    printf("  xor a0, a0, a1\n");
    if (Nd->Kind == ND_EQ)
      // a0==a1
      // a0=a0^a1, sltiu a0, a0, 1
      // sltiu：当作无符号数比较， a0 小于 1 则将 a0 置 1，小于1 的之有0，则
      // 等于0则置1
      printf("  seqz a0, a0\n");
    else
      // a0!=a1
      // a0=a0^a1, sltu a0, x0, a0
      // sltu： 当作无符号数比较， x0 小于 a0 则将 a0 置 1
      // 不等于0则置1
      printf("  snez a0, a0\n");
    return;
  case ND_LT:
    printf("  slt a0, a0, a1\n");
    return;
  case ND_LE:
    // a0<=a1等价于
    // a0=a1<a0, a0=a1^1
    printf("  slt a0, a1, a0\n");
    printf("  xori a0, a0, 1\n");
    return;
  default:
    break;
  }

  error("invalid expression");
}

static void genStmt(Node *Nd)
{
  switch (Nd->Kind)
  {
  //  gen '{}' code block and for-loop the statement link
  case ND_BLOCK:
    for (Node *N = Nd->Body; N; N = N->Next)
      genStmt(N);
    return;
  // gen 'return' statement
  case ND_RETURN:
    genExpr(Nd->LHS);
    // 无条件跳转语句， 跳转到.L.return段
    // j offset是 jal x0, offset的别名指令
    printf("  j .L.return\n");
    return;
  // 生成表达式语句
  case ND_EXPR_STMT:
    genExpr(Nd->LHS);
    return;
  default:
    break;
  }
}
// calculate the variable offset from the 'Locals'
static void assignLVarOffsets(Function *Prog)
{
  int Offset = 0;
  // for-loop local variables
  for (Obj *Var = Prog->Locals; Var; Var = Var->Next)
  {

    // assign 8 bytes to each variable
    Offset += 8;

    // set the offset for each variable, is the addr in the stack.
    Var->Offset = -Offset;
  }
  // align the stack to 16 bytes
  Prog->StackSize = alignTo(Offset, 16);
}

// code gen entry function
void codegen(Function *Prog)
{
  assignLVarOffsets(Prog);
  printf("  .globl main\n");
  printf("main:\n");

  // 栈布局
  //-------------------------------// sp
  //              fp                  fp = sp-8
  //-------------------------------// fp = sp-8
  //           local  variables
  //-------------------------------// sp = sp-8-StackSize
  //           expressions calculate
  //-------------------------------//

  // Prologue
  // 将fp压入栈中，保存fp的值
  printf("  addi sp, sp, -8\n");
  printf("  sd fp, 0(sp)\n");
  // 将sp写入fp
  printf("  mv fp, sp\n");

  // 26个字母*8字节=208字节，栈腾出208字节的空间
  printf("  addi sp, sp, -208\n");

  for (Node *N = Prog->Body; N; N = N->Next)
  {
    genStmt(N);
    assert(Depth == 0);
  }

  // Epilogue
  // 输出return段标签
  printf(".L.return:\n");
  // 将fp的值改写回sp
  printf("  mv sp, fp\n");
  // 将最早fp保存的值弹栈，恢复fp。
  printf("  ld fp, 0(sp)\n");
  printf("  addi sp, sp, 8\n");

  printf("  ret\n");
}