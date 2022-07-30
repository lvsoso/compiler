#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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
        if (*P == '+' || *P == '-')
        {
            // op's len == 1
            Cur->Next = newToken(TK_PUNCT, P, P + 1);
            Cur = Cur->Next;
            ++P;
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

int main(int Argc, char **Argv)
{
    if (Argc != 2)
    {
        error("%s: invalid number of arguments", Argv[0]);
    }

    // parsing argv
  CurrentInput = Argv[1];
  Token *Tok = tokenize();

    printf("  .globl main\n");
    printf("main:\n");

    // 传入第一个number
    // strtol(target, remainder， base)
    printf("  li a0, %d\n", getNumber(Tok));
    Tok = Tok->Next;

    // 解析
    while (Tok->Kind != TK_EOF)
    {
        if (equal(Tok, "+"))
        {
            Tok = Tok->Next;
            printf("  addi a0, a0, %d\n", getNumber(Tok));
            Tok = Tok->Next;
            continue;
        }

        if (equal(Tok, "-"))
        {
            Tok = skip(Tok, "-");
            printf("  addi a0, a0, -%d\n", getNumber(Tok));
            Tok = Tok->Next;
            continue;
        }

        error("unexpected character: '%c'\n", Tok->Loc);
    }

    // jalr x0, x1, 0
    printf("  ret\n");
    return 0;
}
