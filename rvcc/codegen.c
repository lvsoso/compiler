#include "rvcc.h"

// 输出文件
static FILE *OutputFile;

// 记录栈深度
static int Depth;

// 用于函数参数的寄存器们
static char *ArgReg[] = {"a0", "a1", "a2", "a3", "a4", "a5"};

// save current funtion
static Obj *CurrentFn;

static void genExpr(Node *Nd);

static void genStmt(Node *Nd);

// output string and newline
static void printLn(char *Fmt, ...)
{
  va_list VA;

  va_start(VA, Fmt);
  vfprintf(OutputFile, Fmt, VA);
  va_end(VA);

  fprintf(OutputFile, "\n");
}

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
  printLn("  # 压栈，将a0的值存入栈顶");
  // sp  = sp - 8
  printLn("  addi sp, sp, -8");
  // pos[sp+0]= a0
  printLn("  sd a0, 0(sp)");
  Depth++;
}

// 弹栈，将sp指向的地址的值，弹出到a1
static void pop(char *Reg)
{
  printLn("  # 弹栈，将栈顶的值存入%s", Reg);
  // reg = pos[sp+0]
  printLn("  ld %s, 0(sp)", Reg);
  // sp = sp + 8
  printLn("  addi sp, sp, 8");
  Depth--;
}

// align N times to the 'Align'
int alignTo(int N, int Align)
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
    if (Nd->Var->IsLocal)
    {
      // 'offset' is compare with 'fp'
      printLn("  # 获取局部变量%s的栈内地址为%d(fp)", Nd->Var->Name,
              Nd->Var->Offset);
      printLn("  addi a0, fp, %d", Nd->Var->Offset);
    }
    else
    {
      printLn("  # 获取全局变量%s的地址", Nd->Var->Name);
      // 获取全局变量的地址
      // 高地址(高20位,31~20位)
      printLn("  lui a0, %%hi(%s)", Nd->Var->Name);
      // 低地址(低12位,19~0位).
      printLn("  addi a0, a0, %%lo(%s)", Nd->Var->Name);
    }

    return;
  case ND_DEREF:
    genExpr(Nd->LHS);
    return;

  case ND_COMMA:
    genExpr(Nd->LHS);
    genAddr(Nd->RHS);
    return;

  // struct member
  case ND_MEMBER:
    genAddr(Nd->LHS);
    printLn("  # 计算成员变量的地址偏移量");
    printLn("  addi a0, a0, %d", Nd->Mem->Offset);
    return;

  default:
    errorTok(Nd->Tok, "not an lvalue");
  }
}

// load the value a0 point to
static void load(Type *Ty)
{
  if (Ty->Kind == TY_ARRAY || Ty->Kind == TY_STRUCT || Ty->Kind == TY_UNION)
  {
    return;
  }

  printLn("  # 读取a0中存放的地址，得到的值存入a0");
  if (Ty->Size == 1)
  {
    printLn("  lb a0, 0(a0)");
  }
  else if (Ty->Size == 2)
  {
    printLn("  lh a0, 0(a0)");
  }
  else if (Ty->Size == 4)
  {
    printLn("  lw a0, 0(a0)");
  }
  else
  {
    printLn("  ld a0, 0(a0)");
  }
}

// type enumerate
enum { I8, I16, I32, I64 };

// get type enumerate value
static int getTypeId(Type *Ty){
    switch (Ty->Kind) {
  case TY_CHAR:
    return I8;
  case TY_SHORT:
    return I16;
  case TY_INT:
    return I32;
  default:
    return I64;
  }
}

// 类型映射表
// 先逻辑左移N位，再算术右移N位，就实现了将64位有符号数转换为64-N位的有符号数
static char i64i8[] = "  # 转换为i8类型\n"
                      "  slli a0, a0, 56\n"
                      "  srai a0, a0, 56";
static char i64i16[] = "  # 转换为i16类型\n"
                       "  slli a0, a0, 48\n"
                       "  srai a0, a0, 48";
static char i64i32[] = "  # 转换为i32类型\n"
                       "  slli a0, a0, 32\n"
                       "  srai a0, a0, 32";

// 所有类型转换表
static char *castTable[10][10] = {
    // clang-format off

    // 被映射到
    // {i8,  i16,    i32,    i64}
    {NULL,   NULL,   NULL,   NULL}, // 从i8转换
    {i64i8,  NULL,   NULL,   NULL}, // 从i16转换
    {i64i8,  i64i16, NULL,   NULL}, // 从i32转换
    {i64i8,  i64i16, i64i32, NULL}, // 从i64转换

    // clang-format on
};

// 类型转换
static void cast(Type *From, Type *To) {
  if (To->Kind == TY_VOID)
    return;

  if (To->Kind == TY_BOOL) {
    printLn("  # 转为bool类型：为0置0，非0置1");
    printLn("  snez a0, a0");
    return;
  }
  
  // 获取类型的枚举值
  int T1 = getTypeId(From);
  int T2 = getTypeId(To);
  if (castTable[T1][T2]) {
    printLn("  # 转换函数");
    printLn("%s", castTable[T1][T2]);
  }
}

// 将栈顶值(为一个地址)存入a0
static void store(Type *Ty)
{
  pop("a1");

  if (Ty->Kind == TY_STRUCT || Ty->Kind == TY_UNION)
  {
    printLn("  # 对%s进行赋值", Ty->Kind == TY_STRUCT ? "结构体" : "联合体");
    for (int I = 0; I < Ty->Size; ++I)
    {
      printLn("  lb a2, %d(a0)", I);
      printLn("  sb a2, %d(a1)", I);
    }
    return;
  }

  printLn("  # 将a0的值，写入到a1中存放的地址");
  if (Ty->Size == 1)
  {
    printLn("  sb a0, 0(a1)");
  }
  else if (Ty->Size == 2)
  {
    printLn("  sh a0, 0(a1)");
  }
  else if (Ty->Size == 4)
  {
    printLn("  sw a0, 0(a1)");
  }
  else
  {
    printLn("  sd a0, 0(a1)");
  }
};

// 生成表达式
static void genExpr(Node *Nd)
{
  // .loc 文件编号 行号
  printLn("  .loc 1 %d", Nd->Tok->LineNo);

  switch (Nd->Kind)
  {
  // 加载数字到a0
  case ND_NUM:
    printLn("  # 将%d加载到a0中", Nd->Val);
    printLn("  li a0, %ld", Nd->Val);
    return;
  // 对寄存器取反
  case ND_NEG:
    genExpr(Nd->LHS);

    // neg a0, a0是 sub a0, x0的别名, a0，即a0=0-a0
    printLn("  # 对a0值进行取反");
    printLn("  neg a0, a0");
    return;
  // 变量
  case ND_VAR:
  case ND_MEMBER:
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
    store(Nd->Ty);
    return;
  case ND_STMT_EXPR:
    for (Node *N = Nd->Body; N; N = N->Next)
    {
      genStmt(N);
    }
    return;
  // type convert
  case ND_CAST:
    genExpr(Nd->LHS);
    cast(Nd->LHS->Ty, Nd->Ty);
    return;
  case ND_NOT:
    genExpr(Nd->LHS);
    printLn("  # 非运算");
    // a0=0则置1，否则为0
    printLn("  seqz a0, a0");
    return;
  case ND_BITNOT:
    genExpr(Nd->LHS);
    printLn("  # 按位取反");
    // 这里的 not a0, a0 为 xori a0, a0, -1 的伪码
    printLn("  not a0, a0");
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
    printLn("\n  # 调用函数%s", Nd->FuncName);
    printLn("  call %s", Nd->FuncName);
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
   char *Suffix = Nd->LHS->Ty->Kind == TY_LONG || Nd->LHS->Ty->Base ? "" : "w";
  switch (Nd->Kind)
  {
  case ND_ADD: // + a0=a0+a1
    printLn("  # a0+a1，结果写入a0");
    printLn("  add%s a0, a0, a1", Suffix);
    return;
  case ND_SUB: // - a0=a0-a1
    printLn("  # a0-a1，结果写入a0");
    printLn("  sub%s a0, a0, a1", Suffix);
    return;
  case ND_MUL: // * a0=a0*a1
    printLn("  # a0×a1，结果写入a0");
    printLn("  mul%s a0, a0, a1", Suffix);
    return;
  case ND_DIV: // / a0=a0/a1
    printLn("  # a0÷a1，结果写入a0");
    printLn("  div%s a0, a0, a1", Suffix);
    return;
  case ND_MOD: // % a0=a0%a1
    printLn("  # a0%%a1，结果写入a0");
    printLn("  rem%s a0, a0, a1", Suffix);
    return;
  case ND_EQ:
  case ND_NE:
    // a0=a0^a1，异或指令
    printLn("  # 判断是否a0%sa1", Nd->Kind == ND_EQ ? "=" : "≠");
    printLn("  xor a0, a0, a1");
    if (Nd->Kind == ND_EQ)
      // a0==a1
      // a0=a0^a1, sltiu a0, a0, 1
      // sltiu：当作无符号数比较， a0 小于 1 则将 a0 置 1，小于1 的之有0，则
      // 等于0则置1
      printLn("  seqz a0, a0");
    else
      // a0!=a1
      // a0=a0^a1, sltu a0, x0, a0
      // sltu： 当作无符号数比较， x0 小于 a0 则将 a0 置 1
      // 不等于0则置1
      printLn("  snez a0, a0");
    return;
  case ND_LT:
    printLn("  # 判断a0<a1");
    printLn("  slt a0, a0, a1");
    return;
  case ND_LE:
    // a0<=a1等价于
    // a0=a1<a0, a0=a1^1
    printLn("  # 判断是否a0≤a1");
    printLn("  slt a0, a1, a0");
    printLn("  xori a0, a0, 1");
    return;
  default:
    break;
  }

  errorTok(Nd->Tok, "invalid expression");
}

static void genStmt(Node *Nd)
{

  // .loc 文件编号 行号
  printLn("  .loc 1 %d", Nd->Tok->LineNo);

  switch (Nd->Kind)
  {
    // if statement
  case ND_IF:
  {
    // code section count
    int C = count();

    printLn("\n# =====分支语句%d==============", C);
    // gen condition expresion
    printLn("\n# Cond表达式%d", C);
    genExpr(Nd->Cond);

    // judge the result if equal to zero and jump to "else" if no equal
    printLn("  # 若a0为0，则跳转到分支%d的.L.else.%d段", C, C);
    printLn(" beqz a0, .L.else.%d", C);

    // gen the statement for match
    printLn("\n# Then语句%d", C);
    genStmt(Nd->Then);

    // jump to end block
    printLn("  # 跳转到分支%d的.L.end.%d段", C, C);
    printLn("  j .L.end.%d", C);

    // else block
    printLn("\n# Else语句%d", C);
    printLn("# 分支%d的.L.else.%d段标签", C, C);
    printLn(".L.else.%d:", C);

    // gen the  statement for no-match
    if (Nd->Els)
      genStmt(Nd->Els);

    // end of current if statement
    printLn("\n# 分支%d的.L.end.%d段标签", C, C);
    printLn(".L.end.%d:", C);

    // fix: commit 15
    return;
  }

  // gen for-loop or while statement
  case ND_FOR:
  {
    // code section count
    int C = count();
    printLn("\n# =====循环语句%d===============", C);

    if (Nd->Init)
    {
      // generate init statement
      printLn("\n# Init语句%d", C);
      genStmt(Nd->Init);
    }

    // head of loop start
    printLn("\n# 循环%d的.L.begin.%d段标签", C, C);
    printLn(".L.begin.%d:", C);

    // handle for-loop condition
    printLn("# Cond表达式%d", C);
    if (Nd->Cond)
    {
      // generate condition loop statement

      genExpr(Nd->Cond);

      // if result match zero, will jump to end
      printLn("  # 若a0为0，则跳转到循环%d的.L.end.%d段", C, C);
      printLn("  beqz a0, .L.end.%d", C);
    }

    // handle body
    printLn("\n# Then语句%d", C);
    genStmt(Nd->Then);
    // handle increase
    if (Nd->Inc)
    {
      printLn("\n# Inc语句%d", C);
      genExpr(Nd->Inc);
    }

    // jump to begin for next time  loop
    printLn("  # 跳转到循环%d的.L.begin.%d段", C, C);
    printLn("  j .L.begin.%d", C);

    // end of for-loop
    printLn("\n# 循环%d的.L.end.%d段标签", C, C);
    printLn(".L.end.%d:", C);

    return;
  }

  //  gen '{}' code block and for-loop the statement link
  case ND_BLOCK:
    for (Node *N = Nd->Body; N; N = N->Next)
      genStmt(N);
    return;
  // gen 'return' statement
  case ND_RETURN:
    printLn("# 返回语句");
    genExpr(Nd->LHS);
    // 无条件跳转语句， 跳转到.L.return段
    // j offset是 jal x0, offset的别名指令
    printLn("  # 跳转到.L.return.%s段", CurrentFn->Name);
    printLn("  j .L.return.%s", CurrentFn->Name);
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
    if (!Fn->IsFunction)
    {
      continue;
    }

    int Offset = 0;
    // for-loop local variables
    for (Obj *Var = Fn->Locals; Var; Var = Var->Next)
    {

      // assign 'size' bytes to each variable
      Offset += Var->Ty->Size;

      // align variable
      Offset = alignTo(Offset, Var->Ty->Align);

      // set the offset for each variable, is the addr in the stack.
      Var->Offset = -Offset;
    }
    // align the stack to 16 bytes
    Fn->StackSize = alignTo(Offset, 16);
  }
}

static void emitData(Obj *Prog)
{
  for (Obj *Var = Prog; Var; Var = Var->Next)
  {
    if (Var->IsFunction)
      continue;

    printLn("  # 数据段标签");
    printLn("  .data");
    // judge if have the initial data
    if (Var->InitData)
    {
      printLn("%s:", Var->Name);
      // show string content
      printLn("  # 字符串字面量");
      for (int I = 0; I < Var->Ty->Size; ++I)
      {
        char C = Var->InitData[I];
        if (isprint(C))
          printLn("  .byte %d\t# %c", C, C);
        else
          printLn("  .byte %d", C);
      }
    }
    else
    {
      printLn("  # 全局段%s", Var->Name);
      printLn("  .globl %s", Var->Name);
      printLn("%s:", Var->Name);
      printLn("  # 零填充%d位", Var->Ty->Size);
      printLn("  .zero %d", Var->Ty->Size);
    }
  }
}

// save integer register's value into stack
static void storeGeneral(int Reg, int Offset, int Size)
{
  printLn("  # 将%s寄存器的值存入%d(fp)的栈地址", ArgReg[Reg], Offset);
  switch (Size)
  {
  case 1:
    printLn("  sb %s, %d(fp)", ArgReg[Reg], Offset);
    return;
  case 2:
    printLn("  sh %s, %d(fp)", ArgReg[Reg], Offset);
    return;
  case 4:
    printLn("  sw %s, %d(fp)", ArgReg[Reg], Offset);
    return;
  case 8:
    printLn("  sd %s, %d(fp)", ArgReg[Reg], Offset);
    return;
  }
  unreachable();
}

// code gen entry function
void emitText(Obj *Prog)
{
  assignLVarOffsets(Prog);

  // 为每个函数单独生成代码
  for (Obj *Fn = Prog; Fn; Fn = Fn->Next)
  {
    if (!Fn->IsFunction || !Fn->IsDefinition)
    {
      continue;
    }

    if (Fn->IsStatic) {
      printLn("\n  # 定义局部%s函数", Fn->Name);
      printLn("  .local %s", Fn->Name);
    } else {
      printLn("\n  # 定义全局%s函数", Fn->Name);
      printLn("  .globl %s", Fn->Name);
    }
    printLn("  # 代码段标签");
    printLn("  .text");
    printLn("# =====%s段开始===============", Fn->Name);
    printLn("# %s段标签", Fn->Name);
    printLn("%s:", Fn->Name);
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
    printLn("  # 将ra寄存器压栈,保存ra的值");
    printLn("  addi sp, sp, -16");
    printLn("  sd ra, 8(sp)");
    // 将fp压入栈中，保存fp的值
    printLn("  # 将fp压栈，fp属于“被调用者保存”的寄存器，需要恢复原值");
    printLn("  sd fp, 0(sp)");

    // 将sp写入fp
    printLn("  # 将sp的值写入fp");
    printLn("  mv fp, sp");

    printLn("  # sp腾出StackSize大小的栈空间");
    printLn("  addi sp, sp, -%d", Fn->StackSize);

    int I = 0;
    for (Obj *Var = Fn->Params; Var; Var = Var->Next)
    {
      storeGeneral(I++, Var->Offset, Var->Ty->Size);
    }

    // 生成语句链表的代码
    printLn("# =====%s段主体===============", Fn->Name);
    genStmt(Fn->Body);
    assert(Depth == 0);

    // Epilogue
    // 输出return段标签
    printLn("# =====%s段结束===============", Fn->Name);
    printLn("# return段标签");
    printLn(".L.return.%s:", Fn->Name);

    // 将fp的值改写回sp
    printLn("  # 将fp的值写回sp");
    printLn("  mv sp, fp");

    // 将最早fp保存的值弹栈，恢复fp。
    printLn("  # 将最早fp保存的值弹栈，恢复fp和sp");
    printLn("  ld fp, 0(sp)");

    // 将ra寄存器弹栈,恢复ra的值
    printLn("  # 将ra寄存器弹栈,恢复ra的值");
    printLn("  ld ra, 8(sp)");
    printLn("  addi sp, sp, 16");

    //   返回
    printLn("  # 返回a0值给系统调用");
    printLn("  ret");
  }
}

void codegen(Obj *Prog, FILE *Out)
{

  // 设置目标文件的文件流指针
  OutputFile = Out;

  // 计算局部变量的偏移量
  assignLVarOffsets(Prog);
  // 生成数据
  emitData(Prog);
  // 生成代码
  emitText(Prog);
}
