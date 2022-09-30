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
    exit(-1);
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

// Consume the Token
bool consume(Token **Rest, Token *Tok, char *Str)
{
    // exist
    if (equal(Tok, Str)) {
        *Rest = Tok->Next;
        return true;
    }
    // no exist
    *Rest = Tok;
    return false;
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

//  return  a hexadecimal number's decimal value
// hexDigit = [0-9a-fA-F]
// 16: 0 1 2 3 4 5 6 7 8 9  A  B  C  D  E  F
// 10: 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
static int fromHex(char C)
{
    if ('0' <= C && C <= '9')
    {
        return C - '0';
    }
    if ('a' <= C && C <= 'f')
    {
        return C - 'a' + 10;
    }
    return C - 'A' + 10;
}

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
    static char *KW[] = {"return", "if", "else", "for", "while", "int", "sizeof", "char"};

    // for-loop the keyword list and check
    for (int l = 0; l < sizeof(KW)/ sizeof(*KW); ++l) {
        if(equal(Tok, KW[l]))
        return true;
    }

    return false;
}


// read escaped char
static int readEscapedChar(char **NewPos, char *P){
    if('0' <= *P && *P <= '7'){
        // read octonary number which length small than 3;
        // \abc = (a * b + b) * 8 + c
        int C = *P++ - '0';
        if ('0' <= *P && *P <= '7')
        {
            C = (C << 3) + (*P ++ - '0');
            if ('0' <= *P && *P <= '7')
            {
                C = (C << 3) + (*P ++ - '0');
            }
        }
        *NewPos = P;
        return C;
    }

    if (*P == 'x') {
        P ++;
        // is a hexadecimal number
        if (!isxdigit(*P)){
             errorAt(P, "invalid hex escape sequence");
        }

        int C = 0 ;
        // read hexadecimal  which length small than 3;
        // \xWXYZ = ((W*16+X)*16+Y)*16+Z
        for (; isxdigit(*P); P++)
        {
            C = (C << 4) + fromHex(*P);
        }
        *NewPos = P;
        return C;
    }
  *NewPos = P + 1;

  switch (*P) {
  case 'a': // alarm
    return '\a';
  case 'b': // backspace
    return '\b';
  case 't': // tab
    return '\t';
  case 'n': // new line
    return '\n';
  case 'v': // vtab
    return '\v';
  case 'f': // new page
    return '\f';
  case 'r': // return
    return '\r';
  // GNU C extention
  case 'e': // shift
    return 27;
  default: // return raw char
    return *P;
  }
}

// read string literal until end
static char *stringLiteralEnd(char *P)
{
    char *Start = P;
    for (; *P != '"'; P++) {
        if (*P == '\n' || *P == '\0')
        {
             errorAt(Start, "unclosed string literal");
        }
        if (*P == '\\')
        {
            P++;
        }
    }
    return P;
}

// read string literals
static Token *readStringLiteral(char *Start){
    // get end
    char *End = stringLiteralEnd(Start + 1);

  // alloc buffer for the  length of string literal plus one for store
  char *Buf = calloc(1, End - Start);
  int Len = 0;

  for (char *P = Start + 1; P < End;) {
    if (*P == '\\') {
      Buf[Len++] = readEscapedChar(&P, P + 1);
    } else {
      Buf[Len++] = *P++;
    }
  }

  //  create string literal with double quote
  Token *Tok = newToken(TK_STR, Start, End + 1);

    // remain for '\0'
    Tok->Ty = arrayOf(TyChar, Len + 1);
    Tok->Str = Buf;
    return Tok;
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

        // parser string literals
        if (*P == '"') {
        Cur->Next = readStringLiteral(P);
        Cur = Cur->Next;
        P += Cur->Len;
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