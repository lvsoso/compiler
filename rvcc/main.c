#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


//
// 标记符分析，词法分析
//

// Token Type
typedef enum
{
    TK_PUNCT, // +, -
    TK_NUM,   // number
    TK_EOF,   // end
} TokenKind;

// Token
typedef struct Token Token;
struct Token
{
    TokenKind Kind;
    Token *Next;

    int Val;
    char *Loc; // position
    int Len;
};

static char *CurrentInput;

// error struct for output
// static means can be access in file
// Fmt is a format str,
//  "..." means variable parameter .
static void error(char *Fmt, ...)
{

    // 定义一个va_list型的变量存储可变参数
    va_list VA;

    // 指向开始位置
    va_start(VA, Fmt);

    vfprintf(stderr, Fmt, VA);

    // 加一个换行
    fprintf(stderr, "\n");

    // 结束变参获取；
    va_end(VA);

    // exit
    exit(1);
}


// ouput the position and exit.
static void verrorAt(char*Loc, char *Fmt, va_list VA)
{
    fprintf(stderr, "%s\n", CurrentInput);

    // 计算出错的位置，Loc是出错位置的指针，CurrentInput是当前输入的首地址
    // 补充空格将起始符号移动到出错位置
    int Pos = Loc - CurrentInput;
    fprintf(stderr, "%*s", Pos, "");
    fprintf(stderr, "^ ");
    vfprintf(stderr, Fmt, VA);
    fprintf(stderr, "\n");
    va_end(VA);
    exit(1);

}

// 字符解析出错
static void errorAt(char *Loc, char *Fmt, ...)
{
    va_list VA;
    va_start(VA, Fmt);
    verrorAt(Loc, Fmt, VA);
}

// Tok解析出错
static void errorTok(Token *Tok, char *Fmt, ...) {
  va_list VA;
  va_start(VA, Fmt);
  verrorAt(Tok->Loc, Fmt, VA);
}

// judge Tok's value if equal to Str
static bool equal(Token *Tok, char *Str)
{
    return memcmp(Tok->Loc, Str, Tok->Len) == 0 && Str[Tok->Len] == '\0';
}

// skip Str
static Token *skip(Token *Tok, char *Str)
{
    if (!equal(Tok, Str))
        errorTok(Tok, "expect '%s'", Str);
    return Tok->Next;
}

// get number value from token
static int getNumber(Token *Tok)
{
    if (Tok->Kind != TK_NUM)
        errorTok(Tok, "expect a number");
    return Tok->Val;
}

// new Token
static Token *newToken(TokenKind Kind, char *Start, char *End)
{
    Token * Tok = calloc(1, sizeof(Token));
    Tok->Kind = Kind;
    Tok->Loc = Start;
    Tok->Len = End - Start;
    return Tok;
}

// judge a string start with sub-string
static bool startsWith(char *Str, char *SubStr){
  return strncmp(Str, SubStr, strlen(SubStr)) == 0;
}

// read operator
static int readPunct(char *Ptr) {
  // 2 bytes
  if (startsWith(Ptr, "==") || startsWith(Ptr, "!=") || startsWith(Ptr, "<=") || startsWith(Ptr, ">=")){
    return 2;
  }

  // 1 byte
  return ispunct(*Ptr) ? 1 : 0;
}

// 终结符解析
static Token *tokenize()
{
    char *P = CurrentInput;
    Token Head = {};
    Token *Cur = &Head;

    while (*P)
    {
        // 跳过所有空白符如：空格、回车
        if (isspace(*P))
        {
            ++P;
            continue;
        }

        // 解析数字
        if (isdigit(*P))
        {
            Cur->Next = newToken(TK_NUM, P, P);
            Cur = Cur->Next;
            const char *OldPtr = P;
            // here move the  pointer
            Cur->Val = strtoul(P, &P, 10);
            Cur->Len = P - OldPtr;
            continue;
        }

        //  parsing op
        int PunctLen = readPunct(P);
        if (PunctLen) {
          Cur->Next = newToken(TK_PUNCT, P, P + PunctLen);
          Cur = Cur->Next;
          // move the pointer
          P += PunctLen;
          continue;
        }

        // unknown character
        errorAt(P, "invalid token");
    }

    // end of the parse
    Cur->Next = newToken(TK_EOF, P, P);
    // Head is empty
    return Head.Next;
}

//
// 生成AST（抽象语法树），语法解析
//

// AST Nnode
typedef enum {
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_NUM, // interger
    ND_NEG, // 负号 -
    ND_EQ,  // ==
    ND_NE,  // !=
    ND_LT,  // <
    ND_LE,  // <=
} NodeKind;

// AST binary tree's node
typedef struct Node Node;
struct  Node
{
    NodeKind Kind; // node type
    Node *LHS; // left-hand side
    Node *RHS; // right-hand side
    int Val; // value of 'ND_NUM' type
};


// new a node
static Node *newNode(NodeKind Kind){
    Node *Nd = calloc(1, sizeof(Node));
    Nd->Kind = Kind;
    return Nd;
}

// new single tree node
static Node *newUnary(NodeKind Kind, Node *Expr) {
  Node *Nd = newNode(Kind);
  Nd->LHS = Expr;
  return Nd;
}

// new a binary node
static Node *newBinary(NodeKind Kind, Node *LHS, Node *RHS) {
  Node *Nd = newNode(Kind);
  Nd->LHS = LHS;
  Nd->RHS = RHS;
  return Nd;
}

//  new a number node
static Node *newNum(int Val) {
    Node *Nd = newNode(ND_NUM);
    Nd->Val = Val;
    return Nd;
}

// expr = equality
// equality = relational ("==" relational | "!=" relational)*
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
// add = mul ("+" mul | "-" mul)*
// mul = unary ("*" unary | "/" unary)*
// unary = ("+" | "-") unary | primary
// primary = "(" expr ")" | num
static Node *expr(Token **Rest, Token *Tok);
static Node *equality(Token **Rest, Token *Tok);
static Node *relational(Token **Rest, Token *Tok);
static Node *add(Token **Rest, Token *Tok);
static Node *mul(Token **Rest, Token *Tok);
static Node *unary(Token **Rest, Token *Tok);
static Node *primary(Token **Rest, Token *Tok);

// expr = equality
static Node *expr(Token **Rest, Token *Tok) { 
  return equality(Rest, Tok); 
}

// equality = relational ("==" relational | "!=" relational)*
static Node *equality(Token **Rest, Token *Tok) {
  // relational
  Node *Nd = relational(&Tok, Tok);

  // ("==" relational | "!=" relational)*
  while (true) {
    // "==" relational
    if (equal(Tok, "==")) {
      Nd = newBinary(ND_EQ, Nd, relational(&Tok, Tok->Next));
      continue;
    }

    // "!=" relational
    if (equal(Tok, "!=")) {
      Nd = newBinary(ND_NE, Nd, relational(&Tok, Tok->Next));
      continue;
    }

    *Rest = Tok;
    return Nd;
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node *relational(Token **Rest, Token *Tok) {
  // add
  Node *Nd = add(&Tok, Tok);

  // ("<" add | "<=" add | ">" add | ">=" add)*
  while (true) {
    // "<" add
    if (equal(Tok, "<")) {
      Nd = newBinary(ND_LT, Nd, add(&Tok, Tok->Next));
      continue;
    }

    // "<=" add
    if (equal(Tok, "<=")) {
      Nd = newBinary(ND_LE, Nd, add(&Tok, Tok->Next));
      continue;
    }

    // ">" add
    // X>Y  ~ Y<X
    if (equal(Tok, ">")) {
      Nd = newBinary(ND_LT, add(&Tok, Tok->Next), Nd);
      continue;
    }

    // ">=" add
    // X>=Y ~ Y<=X
    if (equal(Tok, ">=")) {
      Nd = newBinary(ND_LE, add(&Tok, Tok->Next), Nd);
      continue;
    }

    *Rest = Tok;
    return Nd;
  }
}
// add = mul ("+" mul | "-" mul)*
static Node *add(Token **Rest, Token *Tok) {
  // mul
  Node *Nd = mul(&Tok, Tok);

   // ("+" mul | "-" mul)*
  while (true) {
    // "+" mul
    if (equal(Tok, "+")) {
      Nd = newBinary(ND_ADD, Nd, mul(&Tok, Tok->Next));
      continue;
    }

    // "-" mul
    if (equal(Tok, "-")) {
      Nd = newBinary(ND_SUB, Nd, mul(&Tok, Tok->Next));
      continue;
    }

    *Rest = Tok;
    return Nd;
  }
}

// 解析乘除
// mul = unary ("*" unary | "/" unary)*
static Node *mul(Token **Rest, Token *Tok) {
  // unary
  Node *Nd = unary(&Tok, Tok);

  // ("*" unary | "/" unary)*
  while (true) {
    // "*" unary
    if (equal(Tok, "*")) {
      Nd = newBinary(ND_MUL, Nd, unary(&Tok, Tok->Next));
      continue;
    }

    // "/" unary
    if (equal(Tok, "/")) {
      Nd = newBinary(ND_DIV, Nd, unary(&Tok, Tok->Next));
      continue;
    }

    *Rest = Tok;
    return Nd;
  }
}

// unary = ("+" | "-") unary | primary
static Node *unary(Token **Rest, Token *Tok) {
  // "+" unary
  if (equal(Tok, "+"))
    return unary(Rest, Tok->Next);

  // "-" unary
  if (equal(Tok, "-"))
    return newUnary(ND_NEG, unary(Rest, Tok->Next));

  // primary
  return primary(Rest, Tok);
}

// 解析括号、数字
// primary = "(" expr ")" | num
static Node *primary(Token **Rest, Token *Tok) {
  // "(" expr ")"
  if (equal(Tok, "(")) {
    Node *Nd = expr(&Tok, Tok->Next);
    *Rest = skip(Tok, ")");
    return Nd;
  }

  // num
  if (Tok->Kind == TK_NUM) {
    Node *Nd = newNum(Tok->Val);
    *Rest = Tok->Next;
    return Nd;
  }

  errorTok(Tok, "expected an expression");
  return NULL;
}

//
// 语义分析与代码生成
//

// 记录栈深度
static int Depth;

// 压栈，将结果临时压入栈中备用
// sp为栈指针，栈反向向下增长，64位下，8个字节为一个单位，所以sp-8
// 当前栈指针的地址就是sp，将a0的值压入栈
// 不使用寄存器存储的原因是因为需要存储的值的数量是变化的。
static void push(void) {
  // sp  = sp - 8
  printf("  addi sp, sp, -8\n");
  //pos[sp+0]= a0
  printf("  sd a0, 0(sp)\n");
  Depth++;
}

// 弹栈，将sp指向的地址的值，弹出到a1
static void pop(char *Reg) {
    // reg = pos[sp+0]
  printf("  ld %s, 0(sp)\n", Reg);
  // sp = sp + 8
  printf("  addi sp, sp, 8\n");
  Depth--;
}

// 生成表达式
static void genExpr(Node *Nd) {
  switch (Nd->Kind) {
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
  switch (Nd->Kind) {
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

int main(int Argc, char **Argv)
{
    if (Argc != 2)
    {
        error("%s: invalid number of arguments", Argv[0]);
    }

    // parsing argv, generate token stream
  CurrentInput = Argv[1];
  Token *Tok = tokenize();

    //parse token stream
  Node *Node = expr(&Tok, Tok);

  if (Tok->Kind != TK_EOF)
    errorTok(Tok, "extra token");

    printf("  .globl main\n");
    printf("main:\n");

 // traversing  ast tree and generate asm
  genExpr(Node);

    // jalr x0, x1, 0
    printf("  ret\n");

    // 如果栈未清空则报错
    assert(Depth == 0);

    return 0;
}
