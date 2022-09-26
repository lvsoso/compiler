#include "rvcc.h"

// 记录栈深度
static int Depth;

// 用于函数参数的寄存器们
static char *ArgReg[] = {"a0", "a1", "a2", "a3", "a4", "a5"};

// save current funtion
static Obj *CurrentFn;

static void genExpr(Node *Nd);

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
  switch (Nd->Kind)
  {
  case ND_VAR:
  if (Nd->Var->IsLocal){
    // 'offset' is compare with 'fp'
    printf("  # 获取局部变量%s的栈内地址为%d(fp)\n", Nd->Var->Name,
           Nd->Var->Offset);
    printf("  addi a0, fp, %d\n", Nd->Var->Offset);
  }else{
    printf("  # 获取全局变量%s的地址\n", Nd->Var->Name);
    // 获取全局变量的地址
    // 高地址(高20位,31~20位)
    printf("  lui a0, %%hi(%s)\n", Nd->Var->Name);
    // 低地址(低12位,19~0位).
    printf("  addi a0, a0, %%lo(%s)\n", Nd->Var->Name);
  }

    return;
  case ND_DEREF:
    genExpr(Nd->LHS);
    return;
  default:
    errorTok(Nd->Tok, "not an lvalue");
  }
}

// load the value a0 point to
static void load(Type *Ty){
  if(Ty->Kind == TY_ARRAY){
    return ;
  }

  printf("  # 读取a0中存放的地址，得到的值存入a0\n");
  printf("  ld a0, 0(a0)\n");
}

// 将栈顶值(为一个地址)存入a0
static void store(void) {
  pop("a1");
  printf("  # 将a0的值，写入到a1中存放的地址\n");
  printf("  sd a0, 0(a1)\n");
};

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
    load(Nd->Ty);
    return;
  // 解引用
  case ND_DEREF:
    genExpr(Nd->LHS);
    load(Nd->Ty);
    return;
  case ND_ADDR:
    genAddr(Nd->LHS);
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
    // 函数调用
  case ND_FUNCALL:
  {
    int NArgs = 0;

    for (Node *Arg = Nd->Args; Arg; Arg = Arg->Next)
    {
      genExpr(Arg);
      push();
      NArgs++;
    }

    for (int i = NArgs - 1; i >= 0; i--)
      pop(ArgReg[i]);

    // call function
    printf("\n  # 调用函数%s\n", Nd->FuncName);
    printf("  call %s\n", Nd->FuncName);
    return;
  }
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

  errorTok(Nd->Tok, "invalid expression");
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
    printf("  # 跳转到.L.return.%s段\n", CurrentFn->Name);
    printf("  j .L.return.%s\n", CurrentFn->Name);
    return;
  // 生成表达式语句
  case ND_EXPR_STMT:
    genExpr(Nd->LHS);
    return;
  default:
    break;
  }

  errorTok(Nd->Tok, "invalid statement");
}
// calculate the variable offset from the 'Locals'
static void assignLVarOffsets(Obj *Prog)
{
  for (Obj *Fn = Prog; Fn; Fn = Fn->Next)
  {
    if (!Fn->isFunction)
    {
      continue;
    }

    int Offset = 0;
    // for-loop local variables
    for (Obj *Var = Fn->Locals; Var; Var = Var->Next)
    {

      // assign 'size' bytes to each variable
      Offset += Var->Ty->Size;

      // set the offset for each variable, is the addr in the stack.
      Var->Offset = -Offset;
    }
    // align the stack to 16 bytes
    Fn->StackSize = alignTo(Offset, 16);
  }
}

static void emitData(Obj *Prog) {
  for (Obj *Var = Prog; Var; Var = Var->Next) {
    if (Var->isFunction)
      continue;

    printf("  # 数据段标签\n");
    printf("  .data\n");
    printf("  .globl %s\n", Var->Name);
    printf("  # 全局变量%s\n", Var->Name);
    printf("%s:\n", Var->Name);
    printf("  # 零填充%d位\n", Var->Ty->Size);
    printf("  .zero %d\n", Var->Ty->Size);
  }
}

// code gen entry function
void emitText(Obj *Prog)
{
  assignLVarOffsets(Prog);

  // 为每个函数单独生成代码
  for (Obj *Fn = Prog; Fn; Fn = Fn->Next)
  {
    if (!Fn->isFunction)
    {
      continue;
    }

    printf("\n  # 定义全局%s段\n", Fn->Name);
    printf("  .globl %s\n", Fn->Name);

    printf("  # 代码段标签\n");
    printf("  .text\n");
    printf("# =====%s段开始===============\n", Fn->Name);
    printf("# %s段标签\n", Fn->Name);
    printf("%s:\n", Fn->Name);
    CurrentFn = Fn;

    // 栈布局
    //-------------------------------// sp
    //              ra
    //-------------------------------// ra = sp-8
    //              fp
    //-------------------------------// fp = sp-16
    //           local  variables
    //-------------------------------// sp = sp-16-StackSize
    //           expressions calculate
    //-------------------------------//

    // Prologue
    // 将ra寄存器压栈,保存ra的值
    printf("  # 将ra寄存器压栈,保存ra的值\n");
    printf("  addi sp, sp, -16\n");
    printf("  sd ra, 8(sp)\n");
    // 将fp压入栈中，保存fp的值
    printf("  # 将fp压栈，fp属于“被调用者保存”的寄存器，需要恢复原值\n");
    printf("  sd fp, 0(sp)\n");

    // 将sp写入fp
    printf("  # 将sp的值写入fp\n");
    printf("  mv fp, sp\n");

    printf("  # sp腾出StackSize大小的栈空间\n");
    printf("  addi sp, sp, -%d\n", Fn->StackSize);

    int I = 0;
    for (Obj *Var = Fn->Params; Var; Var = Var->Next)
    {
      printf("  # 将%s寄存器的值存入%s的栈地址\n", ArgReg[I], Var->Name);
      printf("  sd %s, %d(fp)\n", ArgReg[I++], Var->Offset);
    }

    // 生成语句链表的代码
    printf("# =====%s段主体===============\n", Fn->Name);
    genStmt(Fn->Body);
    assert(Depth == 0);

    // Epilogue
    // 输出return段标签
    printf("# =====%s段结束===============\n", Fn->Name);
    printf("# return段标签\n");
    printf(".L.return.%s:\n", Fn->Name);

    // 将fp的值改写回sp
    printf("  # 将fp的值写回sp\n");
    printf("  mv sp, fp\n");

    // 将最早fp保存的值弹栈，恢复fp。
    printf("  # 将最早fp保存的值弹栈，恢复fp和sp\n");
    printf("  ld fp, 0(sp)\n");

    // 将ra寄存器弹栈,恢复ra的值
    printf("  # 将ra寄存器弹栈,恢复ra的值\n");
    printf("  ld ra, 8(sp)\n");
    printf("  addi sp, sp, 16\n");

    //   返回
    printf("  # 返回a0值给系统调用\n");
    printf("  ret\n");
  }
}

void codegen(Obj *Prog) {
  // 计算局部变量的偏移量
  assignLVarOffsets(Prog);
  // 生成数据
  emitData(Prog);
  // 生成代码
  emitText(Prog);
}
