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
    ND_EXPR_STMT, // statement
    ND_NUM, // interger

} NodeKind;

// AST binary tree's node
typedef struct Node Node;
struct  Node
{
    NodeKind Kind; // node type
    Node *Next; // next node, next statment
    Node *LHS; // left-hand side
    Node *RHS; // right-hand side
    int Val; // value of 'ND_NUM' type
};


// 语法解析入口函数
Node *parse(Token *Tok);

//
// 语义分析与代码生成
//

// 代码生成入口函数
void codegen(Node *Nd);