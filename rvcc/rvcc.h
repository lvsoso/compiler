// 使用POSIX.1标准
// 使用了strndup函数
//  截断字符串
#define _POSIX_C_SOURCE 200809L


#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

//
// 共用头文件，定义了多个文件间共同使用的函数和数据
//

typedef struct Type Type;
typedef struct Node Node;
typedef struct Member Member;

//
// 字符串
//

char *format(char *Fmt, ...);

//
// 标记符分析，词法分析
//

// Token Type
typedef enum
{
    TK_IDENT, // ident, variable name or function name
    TK_KEYWORD,  // keyword
    TK_PUNCT, // +, -
    TK_STR,     // string literals
    TK_NUM,   // number
    TK_EOF,   // end
} TokenKind;

// Token
typedef struct Token Token;
struct Token
{
    TokenKind Kind;
    Token *Next;

    int64_t Val; // TK_NUM
    char *Loc; // position
    int Len;
    Type *Ty;
    char *Str;

    int LineNo; // 行号
};


// function for error ouput
void error(char *Fmt, ...);
void errorAt(char *Loc, char *Fmt, ...);
void errorTok(Token *Tok, char *Fmt, ...);

// 判断Token与Str的关系
bool equal(Token *Tok, char *Str);
Token *skip(Token *Tok, char *Str);
bool  consume(Token **Rest, Token *Tok, char *Str);

// 词法分析
Token *tokenizeFile(char *Path);

// 指rvcc源文件的某个文件的某一行出了问题，打印出文件名和行号
#define unreachable() error("internal error at %s:%d", __FILE__, __LINE__)

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
    ND_COMMA,     // , 
    ND_MEMBER,    // . struct memeber access
    ND_RETURN, // return
    ND_IF,        // "if
    ND_FOR,  // "for" or "while"
    ND_BLOCK,     // { ... }，code block
    ND_FUNCALL,   //function call
    ND_EXPR_STMT, // statement
    ND_STMT_EXPR, // statement expression
    ND_NUM, // interger
    ND_VAR, // variable
    ND_ADDR,  // get address (&)
    ND_DEREF,  // dereference (*)
} NodeKind;

// AST binary tree's node

typedef struct Obj Obj;

struct Obj {
    Obj *Next; // next obj
    char *Name ; // name of variable
    
    // local variable
    int Offset; //  offset of fp
    Type *Ty; // variable type
    bool IsLocal; // local or global variable

    // function or global variable
    bool IsFunction;
    bool IsDefinition;

    char *InitData;


    Obj *Params; // function Params

    Node *Body; // function body or statement expression
    Obj *Locals; // local variable
    int StackSize; // stack size
};

struct  Node
{
    NodeKind Kind; // node type
    Node *Next; // next node, next statement

    Token *Tok;    // node' s token

    Type *Ty;  // node's type

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
    
    // struct member access
    Member *Mem;


    // 函数调用
    char *FuncName; // 函数名
    Node *Args;     // 函数参数

    Obj *Var;      // save ND_VAR type variable
    int64_t Val; // value of 'ND_NUM' type
};


// 语法解析入口函数
Obj *parse(Token *Tok);

// Types System

// types
typedef enum {
    TY_VOID,   // void
    TY_CHAR,  // char
    TY_SHORT,  // short
    TY_INT, // int
    TY_LONG,   // long
    TY_PTR, // pointer
    TY_FUNC, // function
    TY_ARRAY, // array
    TY_STRUCT, // struct
    TY_UNION, // union
} TypeKind;

struct Type {
    TypeKind Kind; // kind
    int Size; // return by 'sizeof'
    int Align;     // align

    Type *Base; // pointed type

    Token* Name; // type's name, veriable's name; function's name

    // array
    int ArrayLen; // len of array, count of the elements
  
    // struct
    Member *Mems;

    // type of function return
    Type *ReturnTy; 

    Type *Params; //

    Type *Next;
};

// struct member
struct Member {
    Member *Next; // next member
    Type *Ty; // type
    Token *Name; // name
    int Offset; // offset
};

// 声明全局变量，定义再type.c中
extern Type *TyVoid;

extern Type *TyInt;
extern Type *TyShort;
extern Type *TyChar;
extern Type *TyLong;

// judge the 'type' is integer
bool isInteger(Type *TY);

// copy type
Type *copyType(Type *Ty);

// build a pointer type, point to the base type
Type *pointerTo(Type *Base);

// add type to each node under the Node;
void addType(Node *Nd);

// array type
Type *arrayOf(Type *Base, int Size);

// function type
Type *funcType(Type *ReturnTy);

//
// 语义分析与代码生成
//

int alignTo(int N, int Align);

// 代码生成入口函数
void codegen(Obj *Prog, FILE *Out);
