// 使用POSIX.1标准
// 使用了strndup函数
//  截断字符串
#define _POSIX_C_SOURCE 200809L


#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// 共用头文件，定义了多个文件间共同使用的函数和数据
//

//
// 标记符分析，词法分析
//

// Token Type
typedef enum
{
    TK_IDENT, // ident, variable name or function name
    TK_KEYWORD,  // keyword
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


// function for error ouput
void error(char *Fmt, ...);
void errorAt(char *Loc, char *Fmt, ...);
void errorTok(Token *Tok, char *Fmt, ...);
// 判断Token与Str的关系
bool equal(Token *Tok, char *Str);
Token *skip(Token *Tok, char *Str);
// 词法分析
Token *tokenize(char *Input);

//
// 生成AST（抽象语法树），语法解析
//

// AST Nnode
typedef enum {
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_NEG, // 负号 -
    ND_EQ,  // ==
    ND_NE,  // !=
    ND_LT,  // <
    ND_LE,  // <=
    ND_ASSIGN, // assign variable's value
    ND_RETURN, // return
    ND_IF,        // "if
    ND_FOR,  // for"
    ND_BLOCK,     // { ... }，code block
    ND_EXPR_STMT, // statement
    ND_NUM, // interger
    ND_VAR, // variable
    
} NodeKind;

// AST binary tree's node
typedef struct Node Node;

typedef struct Obj Obj;

struct Obj {
    Obj *Next; // next obj
    char *Name ; // name of variable
    int Offset ; //  offset of fp
};

// function
typedef struct Function Function;

struct Function {
    Node *Body; // function body
    Obj *Locals; // local variable
    int StackSize; // stack size
};

struct  Node
{
    NodeKind Kind; // node type
    Node *Next; // next node, next statement
    Node *LHS; // left-hand side
    Node *RHS; // right-hand side

    // "if"statement
    Node *Cond; // condition
    Node *Then; // match
    Node *Els;  // no-match


    // for statement
    Node *Init;// init statement
    Node *Inc;  // increase statement

    // code block
    Node *Body;
    
    Obj *Var;      // save ND_VAR type variable
    int Val; // value of 'ND_NUM' type
};


// 语法解析入口函数
Function *parse(Token *Tok);

//
// 语义分析与代码生成
//

// 代码生成入口函数
void codegen(Function *Prog);