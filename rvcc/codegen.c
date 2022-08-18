#include "rvcc.h"

// 记录栈深度
static int Depth;

// code sectionn count
static int count(void)
{
  static int I = 1;
  return I++;
}

// 压栈，将结果临时压入栈中备用
// sp为栈指针，栈反向向下增长，64位下，8个字节为一个单位，所以sp-8
// 当前栈指针的地址就是sp，将a0的值压入栈
// 不使用寄存器存储的原因是因为需要存储的值的数量是变化的。
static void push(void)
{
  printf("  # 压栈，将a0的值存入栈顶\n");
  // sp  = sp - 8
  printf("  addi sp, sp, -8\n");
  // pos[sp+0]= a0
  printf("  sd a0, 0(sp)\n");
  Depth++;
}

// 弹栈，将sp指向的地址的值，弹出到a1
static void pop(char *Reg)
{
  printf("  # 弹栈，将栈顶的值存入%s\n", Reg);
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
    printf("  # 获取变量%s的栈内地址为%d(fp)\n", Nd->Var->Name,
           Nd->Var->Offset);
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
    printf("  # 将%d加载到a0中\n", Nd->Val);
    printf("  li a0, %d\n", Nd->Val);
    return;
  // 对寄存器取反
  case ND_NEG:
    genExpr(Nd->LHS);

    // neg a0, a0是 sub a0, x0的别名, a0，即a0=0-a0
    printf("  # 对a0值进行取反\n");
    printf("  neg a0, a0\n");
    return;
  // 变量
  case ND_VAR:
    // 计算出变量的地址，然后存入a0
    genAddr(Nd);
    // 访问a0地址中存储的数据，存入到a0当中
    printf("  # 读取a0中存放的地址，得到的值存入a0\n");
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
    printf("  # 将a0的值，写入到a1中存放的地址\n");
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
    printf("  # a0+a1，结果写入a0\n");
    printf("  add a0, a0, a1\n");
    return;
  case ND_SUB: // - a0=a0-a1
    printf("  # a0-a1，结果写入a0\n");
    printf("  sub a0, a0, a1\n");
    return;
  case ND_MUL: // * a0=a0*a1
    printf("  # a0×a1，结果写入a0\n");
    printf("  mul a0, a0, a1\n");
    return;
  case ND_DIV: // / a0=a0/a1
    printf("  # a0÷a1，结果写入a0\n");
    printf("  div a0, a0, a1\n");
    return;
  case ND_EQ:
  case ND_NE:
    // a0=a0^a1，异或指令
    printf("  # 判断是否a0%sa1\n", Nd->Kind == ND_EQ ? "=" : "≠");
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
    printf("  # 判断a0<a1\n");
    printf("  slt a0, a0, a1\n");
    return;
  case ND_LE:
    // a0<=a1等价于
    // a0=a1<a0, a0=a1^1
    printf("  # 判断是否a0≤a1\n");
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
    // if statement
  case ND_IF:
  {
    // code section count
    int C = count();

    printf("\n# =====分支语句%d==============\n", C);
    // gen condition expresion
    printf("\n# Cond表达式%d\n", C);
    genExpr(Nd->Cond);

    // judge the result if equal to zero and jump to "else" if no equal
    printf("  # 若a0为0，则跳转到分支%d的.L.else.%d段\n", C, C);
    printf(" beqz a0, .L.else.%d\n", C);

    // gen the statement for match
    printf("\n# Then语句%d\n", C);
    genStmt(Nd->Then);

    // jump to end block
    printf("  # 跳转到分支%d的.L.end.%d段\n", C, C);
    printf("  j .L.end.%d\n", C);

    // else block
    printf("\n# Else语句%d\n", C);
    printf("# 分支%d的.L.else.%d段标签\n", C, C);
    printf(".L.else.%d:\n", C);

    // gen the  statement for no-match
    if (Nd->Els)
      genStmt(Nd->Els);

    // end of current if statement
    printf("\n# 分支%d的.L.end.%d段标签\n", C, C);
    printf(".L.end.%d:\n", C);

    // fix: commit 15
    return;
  }

  // gen for-loop or while statement
  case ND_FOR:
  {
    // code section count
    int C = count();
    printf("\n# =====循环语句%d===============\n", C);

    if (Nd->Init)
    {
      // generate init statement
      printf("\n# Init语句%d\n", C);
      genStmt(Nd->Init);
    }

    // head of loop start
    printf("\n# 循环%d的.L.begin.%d段标签\n", C, C);
    printf(".L.begin.%d:\n", C);

    // handle for-loop condition
    printf("# Cond表达式%d\n", C);
    if (Nd->Cond)
    {
      // generate condition loop statement

      genExpr(Nd->Cond);

      // if result match zero, will jump to end
      printf("  # 若a0为0，则跳转到循环%d的.L.end.%d段\n", C, C);
      printf("  beqz a0, .L.end.%d\n", C);
    }

    // handle body
    printf("\n# Then语句%d\n", C);
    genStmt(Nd->Then);
    // handle increase
    if (Nd->Inc)
    {
      printf("\n# Inc语句%d\n", C);
      genExpr(Nd->Inc);
    }

    // jump to begin for next time  loop
    printf("  # 跳转到循环%d的.L.begin.%d段\n", C, C);
    printf("  j .L.begin.%d\n", C);

    // end of for-loop
    printf("\n# 循环%d的.L.end.%d段标签\n", C, C);
    printf(".L.end.%d:\n", C);

    return;
  }

  //  gen '{}' code block and for-loop the statement link
  case ND_BLOCK:
    for (Node *N = Nd->Body; N; N = N->Next)
      genStmt(N);
    return;
  // gen 'return' statement
  case ND_RETURN:
    printf("# 返回语句\n");
    genExpr(Nd->LHS);
    // 无条件跳转语句， 跳转到.L.return段
    // j offset是 jal x0, offset的别名指令
    printf("  # 跳转到.L.return段\n");
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
  printf("  # 定义全局main段\n");
  printf("  .globl main\n");
  printf("\n# =====程序开始===============\n");
  printf("# main段标签，也是程序入口段\n");
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
  printf("  # 将fp压栈，fp属于“被调用者保存”的寄存器，需要恢复原值\n");
  printf("  addi sp, sp, -8\n");
  printf("  sd fp, 0(sp)\n");

  // 将sp写入fp
  printf("  # 将sp的值写入fp\n");
  printf("  mv fp, sp\n");

  printf("  # sp腾出StackSize大小的栈空间\n");
  printf("  addi sp, sp, -%d\n", Prog->StackSize);

  // 生成语句链表的代码
  printf("\n# =====程序主体===============\n");
  genStmt(Prog->Body);
  assert(Depth == 0);

  // Epilogue
  // 输出return段标签
  printf("\n# =====程序结束===============\n");
  printf("# return段标签\n");
  printf(".L.return:\n");

  // 将fp的值改写回sp
  printf("  # 将fp的值写回sp\n");
  printf("  mv sp, fp\n");

  // 将最早fp保存的值弹栈，恢复fp。
  printf("  # 将最早fp保存的值弹栈，恢复fp和sp\n");
  printf("  ld fp, 0(sp)\n");
  printf("  addi sp, sp, 8\n");

  printf("  # 返回a0值给系统调用\n");
  printf("  ret\n");
}