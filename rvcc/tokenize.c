#include "rvcc.h"

// input filename
static char *CurrentFilename;

// input string
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
// foo.c:10: x = y + 1;
//              ^ <error info>
static void verrorAt(int LineNo,  char *Loc, char *Fmt, va_list VA)
{
    // find line contains loc
    char *Line = Loc;

    // Line递减到当前行的最开始的位置
    // Line<CurrentInput, 判断是否读取到文件最开始的位置
    // Line[-1] != '\n'，Line字符串前一个字符是否为换行符（上一行末尾）
    while (CurrentInput < Line && Line[-1] != '\n')
    {
        Line--;
    }

    // End 递增到行尾的换行符
    char *End = Loc;
    while (*End != '\n')
    {
        End++;
    }

    // 输出 文件名:错误行
    // Indent记录输出了多少个字符
    int Indent = fprintf(stderr, "%s:%d: ", CurrentFilename, LineNo);
    // 输出Line的行内所有字符（不含换行符）
    fprintf(stderr, "%.*s\n", (int)(End - Line), Line);

    // 计算错误信息位置，在当前行内的偏移量+前面输出了多少个字符
    int Pos = Loc - Line + Indent;

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
    int LineNo = 1;
    for (char *P = CurrentInput; P < Loc; P ++)
    {
        if(*P == '\n')
        {
            LineNo ++;
        }
    }
    va_list VA;
    va_start(VA, Fmt);
    verrorAt(LineNo,  Loc, Fmt, VA);
}

// Tok解析出错
void errorTok(Token *Tok, char *Fmt, ...)
{
    va_list VA;
    va_start(VA, Fmt);
    verrorAt(Tok->LineNo, Tok->Loc, Fmt, VA);
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
    if (equal(Tok, Str))
    {
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
    // multi-bytes
    static char *Kw[] = {"==", "!=", "<=", ">=", "->"};

    for (int l = 0; l < sizeof(Kw)/sizeof(*Kw); ++l)
    {
        if(startsWith(Ptr, Kw[l]))
        {
            return strlen(Kw[l]);
        }
    }

    // 1 byte
    return ispunct(*Ptr) ? 1 : 0;
}

// judge if the keyword
static bool isKeyword(Token *Tok)
{
    // keyword list
    static char *KW[] = {"return", "if", "else", "for", "while", "int", "sizeof", "char", "struct"};

    // for-loop the keyword list and check
    for (int l = 0; l < sizeof(KW) / sizeof(*KW); ++l)
    {
        if (equal(Tok, KW[l]))
            return true;
    }

    return false;
}

// read escaped char
static int readEscapedChar(char **NewPos, char *P)
{
    if ('0' <= *P && *P <= '7')
    {
        // read octonary number which length small than 3;
        // \abc = (a * b + b) * 8 + c
        int C = *P++ - '0';
        if ('0' <= *P && *P <= '7')
        {
            C = (C << 3) + (*P++ - '0');
            if ('0' <= *P && *P <= '7')
            {
                C = (C << 3) + (*P++ - '0');
            }
        }
        *NewPos = P;
        return C;
    }

    if (*P == 'x')
    {
        P++;
        // is a hexadecimal number
        if (!isxdigit(*P))
        {
            errorAt(P, "invalid hex escape sequence");
        }

        int C = 0;
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

    switch (*P)
    {
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
    for (; *P != '"'; P++)
    {
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
static Token *readStringLiteral(char *Start)
{
    // get end
    char *End = stringLiteralEnd(Start + 1);

    // alloc buffer for the  length of string literal plus one for store
    char *Buf = calloc(1, End - Start);
    int Len = 0;

    for (char *P = Start + 1; P < End;)
    {
        if (*P == '\\')
        {
            Buf[Len++] = readEscapedChar(&P, P + 1);
        }
        else
        {
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

// add lineno for all token
static void addLineNumbers(Token *Tok)
{
    char *P = CurrentInput;
    int N = 1;

    do {
        if(P == Tok->Loc){
            Tok -> LineNo = N;
            Tok = Tok->Next;
        }
        if (*P == '\n'){
            N++;
            }
    }while (*P++);
}

// 终结符解析, 文件名，文件内容
Token *tokenize(char *Filename, char *P)
{
    CurrentFilename = Filename;
    CurrentInput = P;
    Token Head = {};
    Token *Cur = &Head;

    while (*P)
    {

    // 跳过行注释
    if (startsWith(P, "//"))
    {
      P += 2;
      while (*P != '\n')
    {
        P++;
    }
      continue;
    }

    // 跳过块注释
    if (startsWith(P, "/*"))
    {
      // 查找第一个"*/"的位置
      char *Q = strstr(P + 2, "*/");
      if (!Q)
        {
            errorAt(P, "unclosed block comment");
        }
      P = Q + 2;
      continue;
    }

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
        if (*P == '"')
        {
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

    // add lineno for all token
    addLineNumbers(Head.Next);
    
    // mark all keyword token as KEYWORD
    convertKeywords(Head.Next);

    // Head is empty
    return Head.Next;
}

// read file's contents
static char *readFile(char *Path)
{
    FILE *FP;

    if (strcmp(Path, "-") == 0)
    {
        // if filename is '-', read from input;
        FP = stdin;
    }else{
        FP = fopen(Path, "r");
        if(!FP)
        {
            // errno为系统最后一次的错误代码
            // strerror以字符串的形式输出错误代码
            error("cannot open %s: %s", Path, strerror(errno));
        }
    }

    // return string
    char *Buf;
    size_t BufLen;
    FILE *Out = open_memstream(&Buf, &BufLen);

    // read all file
    while(true)
    {
        char Buf2[4096];

        // fread read data from file-stream to array;
        // array pointer Buf2, size of element is 1, number of element is 4096
        int N = fread(Buf2,1,  sizeof(Buf2), FP);
        if(N == 0)
        {
            break;
        }

        // array pointer Buf2, size of element is 1, real number of element is N
        fwrite(Buf2, 1, N, Out);
    }

    if(FP != stdin){
        fclose(FP);
    }

    // flush output stream buffer
    fflush(Out);

    // confirm the endswith '\n'
    if (BufLen == 0 || Buf[BufLen - 1] != '\n')
    {
        fputc('\n', Out);
    }
    fputc('\0', Out);
    fclose(Out);
    return Buf;
}

Token *tokenizeFile(char *Path) { return tokenize(Path, readFile(Path)); }