#include "rvcc.h"

static char *CurrentInput;

// error struct for output
// static means can be access in file
// Fmt is a format str,
//  "..." means variable parameter .
void error(char *Fmt, ...)
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
static void verrorAt(char *Loc, char *Fmt, va_list VA)
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
void errorAt(char *Loc, char *Fmt, ...)
{
    va_list VA;
    va_start(VA, Fmt);
    verrorAt(Loc, Fmt, VA);
}

// Tok解析出错
void errorTok(Token *Tok, char *Fmt, ...)
{
    va_list VA;
    va_start(VA, Fmt);
    verrorAt(Tok->Loc, Fmt, VA);
}

// judge Tok's value if equal to Str
bool equal(Token *Tok, char *Str)
{
    return memcmp(Tok->Loc, Str, Tok->Len) == 0 && Str[Tok->Len] == '\0';
}

// skip Str
Token *skip(Token *Tok, char *Str)
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
    Token *Tok = calloc(1, sizeof(Token));
    Tok->Kind = Kind;
    Tok->Loc = Start;
    Tok->Len = End - Start;
    return Tok;
}

// judge a string start with sub-string
static bool startsWith(char *Str, char *SubStr)
{
    return strncmp(Str, SubStr, strlen(SubStr)) == 0;
}

// validate the first character of identifier
// [a-zA-Z_]
static bool isIdent1(char C)
{
    return ('a' <= C && C <= 'z') || ('A' <= C && C <= 'Z') || C == '_';
}
// validate  the other character of identifier
// [a-zA-Z0-9_]
static bool isIdent2(char C) { return isIdent1(C) || ('0' <= C && C <= '9'); }

// read operator
static int readPunct(char *Ptr)
{
    // 2 bytes
    if (startsWith(Ptr, "==") || startsWith(Ptr, "!=") || startsWith(Ptr, "<=") || startsWith(Ptr, ">="))
    {
        return 2;
    }

    // 1 byte
    return ispunct(*Ptr) ? 1 : 0;
}

// judge if the keyword
static bool isKeyword(Token *Tok){
    // keyword list
    static char *KW[] = {"return", "if", "else", "for", "while"};

    // for-loop the keyword list and check
    for (int l = 0; l < sizeof(KW)/ sizeof(*KW); ++l) {
        if(equal(Tok, KW[l]))
        return true;
    }

    return false;
}


// convert 'return' to keyword
static void convertKeywords(Token *Tok)
{
    for (Token *T = Tok; T->Kind != TK_EOF; T = T->Next)
    {
        if (isKeyword(T))
        {
            T->Kind = TK_KEYWORD;
        }
    }
}

// 终结符解析
Token *tokenize(char *P)
{
    CurrentInput = P;
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

        // parsing var or keyword
        // [a-zA-Z_][a-zA-Z0-9_]*
        if (isIdent1(*P))
        {
            char *Start = P;
            do
            {
                ++P;
            } while (isIdent2(*P));
            Cur->Next = newToken(TK_IDENT, Start, P);
            Cur = Cur->Next;
            continue;
        }

        //  parsing op
        int PunctLen = readPunct(P);
        if (PunctLen)
        {
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

    // mark all keyword token as KEYWORD
    convertKeywords(Head.Next);

    // Head is empty
    return Head.Next;
}