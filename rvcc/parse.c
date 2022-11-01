#include "rvcc.h"

// local or global variable or typedef scope
typedef struct VarScope VarScope;
struct VarScope
{
  VarScope *Next;
  char *Name;
  Obj *Var;
  Type *Typedef;

  Type *EnumTy;
  int EnumVal;
};

// struct tag and union, enum tag scope
typedef struct TagScope TagScope;
struct TagScope
{
  TagScope *Next; // next tag scope
  char *Name;     // scope name
  Type *Ty;       // scope type
};

typedef struct Scope Scope;
struct Scope
{
  Scope *Next;

  // C有两个域：变量域(变量,类型别名)，结构体标签（结构体、联合体，枚举）域
  VarScope *Vars; // 指向当前域内的变量
  TagScope *Tags; // 指向当前域内的结构体标签
};

// variable attribute
typedef struct
{
  bool IsTypedef;
  bool IsStatic; 
} VarAttr;

// all scope link
static Scope *Scp = &(Scope){};

// function that parsing
static Obj *CurrentFn;

// `goto` and lable in  current function
static Node *Gotos;
static Node *Labels;

static char *BrkLabel;

static char *ContLabel;

// is not empty when parsing "switch" statment
static Node *CurrentSwitch;

// save local variables
Obj *Locals;
// save global variables
Obj *Globals;

/// Some "*" just for the pointer

// program = (typedef | functionDefinition* | global-variable)*
// functionDefinition = declspec declarator? ident "(" ")" "{" compoundStmt*
// declspec = ( "void" | "_Bool" | "char" | "short" | "int" | "long"  | "typedef"  | "static" | structDecl | unionDecl | typedefName
//             | enumSpecifier)+
// enumSpecifier = ident? "{" enumList? "}"
//                 | ident ("{" enumList? "}")?
// enumList = ident ("=" num)? ("," ident ("=" num)?)*
// declarator = "*"* ("(" ident ")" | "(" declarator ")" | ident) typeSuffix
// typeSuffix = "(" funcParams | "[" arrayDimensions | ε
// arrayDimensions = num? "]" typeSuffix
// funcParams = (param ("," param)*)? ")"
// param = declspec declarator
// compoundStmt =  (typedef | declaration | stmt)* "}"
// declaration =
//    declspec (declarator ("=" expr)? ("," declarator ("=" expr)?)*)? ";"
// stmt = "return" expr ";"
//        | "if" "(" expr ")" stmt ("else" stmt )?
//        | "case" num ":" stmt
//        | "default" ":" stmt
//        | "for" "(" exprStmt expr? ";" expr? ")" stmt
//        | "while" "(" expr ")" stmt
//        | "goto" ident ";"
//        | "break" ";"
//        | ident ":" stmt
//        | "{" compoundStmt
//        | exprStmt
// exprStmt = expr? ";"
// expr = assign ("," expr)?
// assign = logOr (assignOp assign)?
// logOr = logAnd ("||" logAnd)*
// logAnd = bitOr ("&&" bitOr)*
// bitOr = bitXor ("|" bitXor)*
// bitXor = bitAnd ("^" bitAnd)*
// bitAnd = equality ("&" equality)*
// assignOp = "=" | "+=" | "-=" | "*=" | "/=" | "%=" | "&=" | "|=" | "^="
//          | "<<=" | ">>="
// equality = relational ("==" relational | "!=" relational)*
// relational = shift ("<" shift | "<=" shift | ">" shift | ">=" shift)*
// shift = add ("<<" add | ">>" add)*
// add = mul ("+" mul | "-" mul)*
// mul = cast ("*" cast | "/" cast | "%" cast)*
// cast = "(" typeName ")" cast | unary
// unary = ("+" | "-" | "*" | "&" | "!" | "~") cast
//       | ("++" | "--") unary
//       | postfix
// structMembers = (declspec declarator (","  declarator)* ";")*
// structDecl = structUnionDecl
// unionDecl = structUnionDecl
// structUnionDecl = ident? ("{" structMembers)?
// postfix = primary ("[" expr "]" | "." ident)* | "->" ident | "++" | "--")*
// primary =  "(" "{" stmt+ "}" ")"
//         | "(" expr ")"
//         | "sizeof" "(" typeName ")"
//         | "sizeof" unary
//         | ident funcArgs?
//         | str
//         | num
// typeName = declspec abstractDeclarator
// abstractDeclarator = "*"* ("(" abstractDeclarator ")")? typeSuffix

// funcall = ident "(" (assign ("," assign)*)? ")"
static bool isTypename(Token *Tok);
static Type *declspec(Token **Rest, Token *Tok, VarAttr *Attr);
static Type *enumSpecifier(Token **Rest, Token *Tok);
static Type *typeSuffix(Token **Rest, Token *Tok, Type *Ty);
static Type *declarator(Token **Rest, Token *Tok, Type *Ty);
static Node *declaration(Token **Rest, Token *Tok, Type *BaseTy);
static Node *compoundStmt(Token **Rest, Token *tok);
static Node *stmt(Token **Rest, Token *Tok);
static Node *exprStmt(Token **Rest, Token *Tok);
static Node *expr(Token **Rest, Token *Tok);
static Node *assign(Token **Rest, Token *Tok);
static Node *logOr(Token **Rest, Token *Tok);
static Node *logAnd(Token **Rest, Token *Tok);
static Node *bitOr(Token **Rest, Token *Tok);
static Node *bitXor(Token **Rest, Token *Tok);
static Node *bitAnd(Token **Rest, Token *Tok);
static Node *equality(Token **Rest, Token *Tok);
static Node *relational(Token **Rest, Token *Tok);
static Node *shift(Token **Rest, Token *Tok);
static Node *add(Token **Rest, Token *Tok);
static Node *newAdd(Node *LHS, Node *RHS, Token *Tok);
static Node *newSub(Node *LHS, Node *RHS, Token *Tok);
static Node *mul(Token **Rest, Token *Tok);
static Node *cast(Token **Rest, Token *Tok);
static Type *structDecl(Token **Rest, Token *Tok);
static Type *unionDecl(Token **Rest, Token *Tok);
static Node *unary(Token **Rest, Token *Tok);
static Node *postfix(Token **Rest, Token *Tok);
static Node *primary(Token **Rest, Token *Tok);
static Token *parseTypedef(Token *Tok, Type *BaseTy);

// Enter scope
static void enterScope(void)
{
  Scope *S = calloc(1, sizeof(Scope));
  S->Next = Scp;
  Scp = S;
}

//  exit scope
static void leaveScope(void) { Scp = Scp->Next; }

// find local variable by name
static VarScope *findVar(Token *Tok)
{
  // find from scope first, deepin first
  for (Scope *S = Scp; S; S = S->Next)
  {
    // for-loop all variable in scope
    for (VarScope *S2 = S->Vars; S2; S2 = S2->Next)
    {
      if (equal(Tok, S2->Name))
      {
        return S2;
      }
    }
  }
  return NULL;
}

// find tag by token
static Type *findTag(Token *Tok)
{
  for (Scope *S = Scp; S; S = S->Next)
  {
    for (TagScope *S2 = S->Tags; S2; S2 = S2->Next)
    {
      if (equal(Tok, S2->Name))
      {
        return S2->Ty;
      }
    }
  }
  return NULL;
}

// new a node
static Node *newNode(NodeKind Kind, Token *Tok)
{
  Node *Nd = calloc(1, sizeof(Node));
  Nd->Kind = Kind;
  Nd->Tok = Tok;
  return Nd;
}

// new single tree node
static Node *newUnary(NodeKind Kind, Node *Expr, Token *Tok)
{
  Node *Nd = newNode(Kind, Tok);
  Nd->LHS = Expr;
  return Nd;
}

// new a binary node
static Node *newBinary(NodeKind Kind, Node *LHS, Node *RHS, Token *Tok)
{
  Node *Nd = newNode(Kind, Tok);
  Nd->LHS = LHS;
  Nd->RHS = RHS;
  return Nd;
}

//  new a number node
static Node *newNum(int64_t Val, Token *Tok)
{
  Node *Nd = newNode(ND_NUM, Tok);
  Nd->Val = Val;
  return Nd;
}

// new a long int node
static Node *newLong(int64_t Val, Token *Tok)
{
  Node *Nd = newNode(ND_NUM, Tok);
  Nd->Val = Val;
  Nd->Ty = TyLong;
  return Nd;
}

// new variable
static Node *newVarNode(Obj *Var, Token *Tok)
{
  Node *Nd = newNode(ND_VAR, Tok);
  Nd->Var = Var;
  return Nd;
}
// new Convert
Node *newCast(Node *Expr, Type *Ty)
{
  addType(Expr);

  Node *Nd = calloc(1, sizeof(Node));
  Nd->Kind = ND_CAST;
  Nd->Tok = Expr->Tok;
  Nd->LHS = Expr;
  Nd->Ty = copyType(Ty);
  return Nd;
}

// save variable in current scope
static VarScope *pushScope(char *Name)
{
  VarScope *S = calloc(1, sizeof(VarScope));
  S->Name = Name;

  // append to the head of linker;
  S->Next = Scp->Vars;
  Scp->Vars = S;
  return S;
}

// new variable
static Obj *newVar(char *Name, Type *Ty)
{
  Obj *Var = calloc(1, sizeof(Obj));
  Var->Name = Name;
  Var->Ty = Ty;
  pushScope(Name)->Var = Var;
  return Var;
}

// insert new variable to 'locals'
static Obj *newLVar(char *Name, Type *Ty)
{
  Obj *Var = newVar(Name, Ty);
  Var->IsLocal = true;
  // the new one be the head
  Var->Next = Locals;
  Locals = Var;
  return Var;
}

// add new global variable
static Obj *newGVar(char *Name, Type *Ty)
{
  Obj *Var = newVar(Name, Ty);
  Var->Next = Globals;
  Globals = Var;
  return Var;
}

// new unique name
static char *newUniqueName(void)
{
  static int Id = 0;
  return format(".L..%d", Id++);
}

// anonymous global varibale
static Obj *newAnonGVar(Type *Ty) { return newGVar(newUniqueName(), Ty); }

// new string literal
static Obj *newStringLiteral(char *Str, Type *Ty)
{
  Obj *Var = newAnonGVar(Ty);
  Var->InitData = Str;
  return Var;
}

// get ident
static char *getIdent(Token *Tok)
{
  if (Tok->Kind != TK_IDENT)
  {
    errorTok(Tok, "expected an identifier");
  }
  return strndup(Tok->Loc, Tok->Len);
}

// find type name
static Type *findTypedef(Token *Tok)
{
  if (Tok->Kind == TK_IDENT)
  {
    VarScope *S = findVar(Tok);
    if (S)
      return S->Typedef;
  }
  return NULL;
}

// get number
static int64_t getNumber(Token *Tok)
{
  if (Tok->Kind != TK_NUM)
  {
    errorTok(Tok, "expected a number");
  }
  return Tok->Val;
}

static void pushTagScope(Token *Tok, Type *Ty)
{
  TagScope *S = calloc(1, sizeof(TagScope));
  S->Name = strndup(Tok->Loc, Tok->Len);
  S->Ty = Ty;
  S->Next = Scp->Tags;
  Scp->Tags = S;
}

// declspec =  ("void" | "_Bool" | "char" | "short" | "int"  | "long"
//        | "typedef" | "static" | structDecl | unionDecl| typedefName
//             | enumSpecifier)+
// declarator specifier
static Type *declspec(Token **Rest, Token *Tok, VarAttr *Attr)
{
  // type's combination: long + long = 1<<9
  // long int == int long
  enum
  {
    VOID = 1 << 0,
    BOOL = 1 << 2,
    CHAR = 1 << 4,
    SHORT = 1 << 6,
    INT = 1 << 8,
    LONG = 1 << 10,
    OTHER = 1 << 12,
  };

  Type *Ty = TyInt;
  int Counter = 0; //

  while (isTypename(Tok))
  {
    if (equal(Tok, "typedef") || equal(Tok, "static")) 
    {
      if (!Attr)
      {
        errorTok(Tok, "storage class specifier is not allowed in this context");
      }
      if (equal(Tok, "typedef")){
        Attr->IsTypedef = true;
      }
      else{
        Attr->IsStatic = true;
      }

      if (Attr->IsTypedef && Attr->IsStatic){
        errorTok(Tok, "typedef and static may not be used together");
      }

      Tok = Tok->Next;
      continue;
    }
    Type *Ty2 = findTypedef(Tok);

    if (equal(Tok, "struct") || equal(Tok, "union") || equal(Tok, "enum") ||
        Ty2) {
      if (Counter)
      {
        break;
      }

      if (equal(Tok, "struct"))
      {
        Ty = structDecl(&Tok, Tok->Next);
      }
      else if (equal(Tok, "union"))
      {
        Ty = unionDecl(&Tok, Tok->Next);
      } 
      else if (equal(Tok, "enum")) {
        Ty = enumSpecifier(&Tok, Tok->Next);
      } 
      else
      {
        Ty = Ty2;
        Tok = Tok->Next;
      }
      Counter += OTHER;
      continue;
    }

    if (equal(Tok, "void"))
    {
      Counter += VOID;
    }
    else if (equal(Tok, "_Bool"))
    {
      Counter += BOOL;
    }
    else if (equal(Tok, "char"))
    {
      Counter += CHAR;
    }
    else if (equal(Tok, "short"))
    {
      Counter += SHORT;
    }
    else if (equal(Tok, "int"))
    {
      Counter += INT;
    }
    else if (equal(Tok, "long"))
    {
      Counter += LONG;
    }
    else
    {
      unreachable();
    }
    switch (Counter)
    {
    case VOID:
      Ty = TyVoid;
      break;
    case BOOL:
      Ty = TyBool;
      break;
    case CHAR:
      Ty = TyChar;
      break;
    case SHORT:
    case SHORT + INT:
      Ty = TyShort;
      break;
    case INT:
      Ty = TyInt;
      break;
    case LONG:
    case LONG + INT:
    case LONG + LONG:
    case LONG + LONG + INT:
      Ty = TyLong;
      break;
    default:
      errorTok(Tok, "invalid type");
    }

    Tok = Tok->Next;
  }
  *Rest = Tok;
  return Ty;
}
// funcParams = (param ("," param)*)? ")"
// param = declspec declarator
static Type *funcParams(Token **Rest, Token *Tok, Type *Ty)
{
  Type Head = {};
  Type *Cur = &Head;

  while (!equal(Tok, ")"))
  {
    // funcParams = param ("," param)*
    // param = declspec declarator
    if (Cur != &Head){
      Tok = skip(Tok, ",");
    }
    
    Type *Ty2 = declspec(&Tok, Tok, NULL);
    Ty2 = declarator(&Tok, Tok, Ty2);

    // T类型的数组被转换为T*
    if (Ty2->Kind == TY_ARRAY) {
      Token *Name = Ty2->Name;
      Ty2 = pointerTo(Ty2->Base);
      Ty2->Name = Name;
    }

    // 将类型复制到形参链表一份
    Cur->Next = copyType(Ty2);

    Cur = Cur->Next;
  }

  // package a func type
  Ty = funcType(Ty);
  // pass the args
  Ty->Params = Head.Next;
  *Rest = Tok->Next;
  return Ty;
}

// arrayDimensions = num? "]" typeSuffix
static Type *arrayDimensions(Token **Rest, Token *Tok, Type *Ty) {
  // "[]" empty
  if (equal(Tok, "]")) {
    Ty = typeSuffix(Rest, Tok->Next, Ty);
    return arrayOf(Ty, -1);
  }

  // have dimension
  int Sz = getNumber(Tok);
  Tok = skip(Tok->Next, "]");
  Ty = typeSuffix(Rest, Tok, Ty);
  return arrayOf(Ty, Sz);
}

// typeSuffix = "(" funcParams | "[" arrayDimensions | ε
static Type *typeSuffix(Token **Rest, Token *Tok, Type *Ty)
{
   // "(" funcParams
  if (equal(Tok, "(")){
    return funcParams(Rest, Tok->Next, Ty);
  }
  
  // "[" arrayDimensions
  if (equal(Tok, "[")){
    return arrayDimensions(Rest, Tok->Next, Ty);
  }

  *Rest = Tok;
  return Ty;
}
// declarator = "*"* ident typeSuffix
static Type *declarator(Token **Rest, Token *Tok, Type *Ty)
{
  // "*"*
  // 构建所有的（多重）指针
  while (consume(&Tok, Tok, "*"))
  {
    Ty = pointerTo(Ty);
  }

  // "(" declarator ")"
  if (equal(Tok, "("))
  {
    // log"("的位置
    Token *Start = Tok;
    Type Dummy = {};
    // 使Tok前进到")"后面的位置
    declarator(&Tok, Start->Next, &Dummy);
    Tok = skip(Tok, ")");
    // 获取到括号后面的类型后缀，Ty为解析完的类型，Rest指向分号
    Ty = typeSuffix(Rest, Tok, Ty);
    // 解析Ty整体作为Base去构造，返回Type的值
    return declarator(&Tok, Start->Next, Ty);
  }

  if (Tok->Kind != TK_IDENT)
  {
    errorTok(Tok, "expected a variable name");
  }

  // typeSuffix
  Ty = typeSuffix(Rest, Tok->Next, Ty);

  // ident
  // 变量名
  Ty->Name = Tok;

  return Ty;
}

// abstractDeclarator = "*"* ("(" abstractDeclarator ")")? typeSuffix
static Type *abstractDeclarator(Token **Rest, Token *Tok, Type *Ty)
{
  // "*"*
  while (equal(Tok, "*"))
  {
    Ty = pointerTo(Ty);
    Tok = Tok->Next;
  }

  // ("(" abstractDeclarator ")")?
  if (equal(Tok, "("))
  {
    Token *Start = Tok;
    Type Dummy = {};
    // 使Tok前进到")"后面的位置
    abstractDeclarator(&Tok, Start->Next, &Dummy);
    Tok = skip(Tok, ")");
    // 获取到括号后面的类型后缀，Ty为解析完的类型，Rest指向分号
    Ty = typeSuffix(Rest, Tok, Ty);
    // 解析Ty整体作为Base去构造，返回Type的值
    return abstractDeclarator(&Tok, Start->Next, Ty);
  }

  // typeSuffix
  return typeSuffix(Rest, Tok, Ty);
}

// typeName = declspec abstractDeclarator
// get type info
static Type *typename(Token **Rest, Token *Tok) 
{
  // declspec
  Type *Ty = declspec(&Tok, Tok, NULL);
  // abstractDeclarator
  return abstractDeclarator(Rest, Tok, Ty);
}

// enumSpecifier = ident? "{" enumList? "}"
//               | ident ("{" enumList? "}")?
// enumList      = ident ("=" num)? ("," ident ("=" num)?)*
static Type *enumSpecifier(Token **Rest, Token *Tok) {
  Type *Ty = enumType();

  // ident?
  Token *Tag = NULL;
  if (Tok->Kind == TK_IDENT) {
    Tag = Tok;
    Tok = Tok->Next;
  }

  if (Tag && !equal(Tok, "{")) {
    Type *Ty = findTag(Tag);
    if (!Ty)
      errorTok(Tag, "unknown enum type");
    if (Ty->Kind != TY_ENUM)
      errorTok(Tag, "not an enum tag");
    *Rest = Tok;
    return Ty;
  }

  // "{" enumList? "}"
  Tok = skip(Tok, "{");

  // enumList
  int I = 0; 
  int Val = 0;
  while (!equal(Tok, "}")) {
    if (I++ > 0)
      Tok = skip(Tok, ",");

    char *Name = getIdent(Tok);
    Tok = Tok->Next;

    if (equal(Tok, "=")) {
      Val = getNumber(Tok->Next);
      Tok = Tok->Next->Next;
    }

    VarScope *S = pushScope(Name);
    S->EnumTy = Ty;
    S->EnumVal = Val++;
  }

  *Rest = Tok->Next;

  if (Tag)
    pushTagScope(Tag, Ty);
  return Ty;
}

// declaration =
// declspec (declarator ("=" expr) ? ("," declarator("=" expr)?)*)? ";"
static Node *declaration(Token **Rest, Token *Tok, Type *BaseTy)
{
  Node Head = {};
  Node *Cur = &Head;
  // count the "declear" of the variable
  int I = 0;

  // (declarator ("=" expr)? ("," declarator ("=" expr)?)*)?
  while (!equal(Tok, ";"))
  {

    // the first variable no need to match ","
    if (I++ > 0)
      Tok = skip(Tok, ",");

    // declarator
    // get variable's name and type
    Type *Ty = declarator(&Tok, Tok, BaseTy);
    
    if (Ty->Size < 0){
      errorTok(Tok, "variable has incomplete type");
    }

    if (Ty->Kind == TY_VOID)
    {
      errorTok(Tok, "variable declared void");
    }

    Obj *Var = newLVar(getIdent(Ty->Name), Ty);

    // 如果不存在"="则为变量声明，不需要生成节点，已经存储在Locals中了
    // if not exist "=", no need to generate  node
    if (!equal(Tok, "="))
      continue;

    // 解析“=”后面的Token
    // parse the token after "="
    Node *LHS = newVarNode(Var, Ty->Name);

    // 解析递归赋值语句
    // parse recursive assignment statement
    Node *RHS = assign(&Tok, Tok->Next);
    Node *Node = newBinary(ND_ASSIGN, LHS, RHS, Tok);
    // 存放在表达式语句中
    // save to the expression statement
    Cur->Next = newUnary(ND_EXPR_STMT, Node, Tok);
    Cur = Cur->Next;
  }

  // 将所有表达式语句，存放在代码块中
  // save all expression staements into code block
  Node *Nd = newNode(ND_BLOCK, Tok);
  Nd->Body = Head.Next;
  *Rest = Tok->Next;
  return Nd;
}

// 判断是否为类型名
static bool isTypename(Token *Tok)
{
  static char *Kw[] = {
      "void",
      "_Bool",
      "char",
      "short",
      "int",
      "long",
      "struct",
      "union",
      "typedef",
      "enum",
      "static",
  };

  for (int l = 0; l < sizeof(Kw) / sizeof(*Kw); ++l)
  {
    if (equal(Tok, Kw[l]))
    {
      return true;
    }
  }
  return findTypedef(Tok);
}

// stmt = "return" expr ";"
//        | "if" "(" expr ")" stmt ("else" stmt )?
//        | "switch" "(" expr ")" stmt
//        | "case" num ":" stmt
//        | "default" ":" stmt
//        | "for" "(" exprStmt expr? ";" expr? ")" stmt
//        | "while" "(" expr ")" stmt
//        | "goto" ident ";"
//        | "break" ";"
//        | "continue" ";"
//        | ident ":" stmt
//        | "{" compoundStmt
//        | exprStmt
static Node *stmt(Token **Rest, Token *Tok)
{
  // "return" expr ";"
  if (equal(Tok, "return"))
  {
    Node *Nd = newNode(ND_RETURN, Tok);
    Node *Exp = expr(&Tok, Tok->Next);
    *Rest = skip(Tok, ";");

    addType(Exp);
    Nd->LHS = newCast(Exp, CurrentFn->Ty->ReturnTy);

    return Nd;
  }

  // "if" "(" expr ")" stmt ("else" stmt)?
  if (equal(Tok, "if"))
  {
    Node *Nd = newNode(ND_IF, Tok);
    // "(" expr ")"，condition
    Tok = skip(Tok->Next, "(");
    Nd->Cond = expr(&Tok, Tok);
    Tok = skip(Tok, ")");
    // stmt，if match condition
    Nd->Then = stmt(&Tok, Tok);
    // ("else" stmt)?, if no-match condition
    if (equal(Tok, "else")){
      Nd->Els = stmt(&Tok, Tok->Next);
    }
    *Rest = Tok;
    return Nd;
  }

// "switch" "(" expr ")" stmt
  if (equal(Tok, "switch")) {
    Node *Nd = newNode(ND_SWITCH, Tok);
    Tok = skip(Tok->Next, "(");
    Nd->Cond = expr(&Tok, Tok);
    Tok = skip(Tok, ")");

    // save before CurrentSwitch
    Node *Sw = CurrentSwitch;

    // save current switch
    CurrentSwitch = Nd;

    // save before break tag's name
    char *Brk = BrkLabel;

    // sat break tag's name
    BrkLabel = Nd->BrkLabel = newUniqueName();

    // parse case
    // stmt
    Nd->Then = stmt(Rest, Tok);

    CurrentSwitch = Sw;
    BrkLabel = Brk;

    return Nd;
  }

  // "case" num ":" stmt
  if (equal(Tok, "case")) {
    if (!CurrentSwitch){
      errorTok(Tok, "stray case");
    }
    // case value
    int Val = getNumber(Tok->Next);

    Node *Nd = newNode(ND_CASE, Tok);
    Tok = skip(Tok->Next->Next, ":");
    Nd->Label = newUniqueName();
    
    // case statments
    Nd->LHS = stmt(Rest, Tok);
    
    // case value
    Nd->Val = Val;

    // save old case head
    Nd->CaseNext = CurrentSwitch->CaseNext;
    
    // save new case
    CurrentSwitch->CaseNext = Nd;

    return Nd;
  }

  // "default" ":" stmt
  if (equal(Tok, "default")) {
    if (!CurrentSwitch){
      errorTok(Tok, "stray default");
    }

    Node *Nd = newNode(ND_CASE, Tok);

    Tok = skip(Tok->Next, ":");

    Nd->Label = newUniqueName();

    Nd->LHS = stmt(Rest, Tok);

    // save default path
    CurrentSwitch->DefaultCase = Nd;
    return Nd;
  }

  // "for" "(" exprStmt expr? ";" expr? ")" stmt
  if (equal(Tok, "for"))
  {
    Node *Nd = newNode(ND_FOR, Tok);
    // "("
    Tok = skip(Tok->Next, "(");

    enterScope();

    // save break lable
    char *Brk = BrkLabel;
    char *Cont = ContLabel;

    BrkLabel = Nd->BrkLabel = newUniqueName();
    ContLabel = Nd->ContLabel = newUniqueName();

    // exprStmt
    if (isTypename(Tok)) {
      // init variable
      Type *BaseTy = declspec(&Tok, Tok, NULL);
      Nd->Init = declaration(&Tok, Tok, BaseTy);
    } else {
      // init statement
      Nd->Init = exprStmt(&Tok, Tok);
    }

    // expr?
    if (!equal(Tok, ";"))
      Nd->Cond = expr(&Tok, Tok);
    // ";"
    Tok = skip(Tok, ";");

    // expr?
    if (!equal(Tok, ")"))
      Nd->Inc = expr(&Tok, Tok);
    // ")"
    Tok = skip(Tok, ")");

    // stmt
    Nd->Then = stmt(Rest, Tok);

    leaveScope();
    
    BrkLabel = Brk;
    ContLabel = Cont;

    return Nd;
  }

  // "while" "(" expr ")" stmt
  if (equal(Tok, "while"))
  {
    Node *Nd = newNode(ND_FOR, Tok);
    // "("
    Tok = skip(Tok->Next, "(");
    // expr
    Nd->Cond = expr(&Tok, Tok);
    // ")"
    Tok = skip(Tok, ")");


    char *Brk = BrkLabel;
    char *Cont = ContLabel;

   BrkLabel = Nd->BrkLabel = newUniqueName();
   ContLabel = Nd->ContLabel = newUniqueName();

    // stmt
    Nd->Then = stmt(Rest, Tok);

    BrkLabel = Brk;
    ContLabel = Cont;

    return Nd;
  }

  // "goto" ident ";"
  if (equal(Tok, "goto")) {
    Node *Nd = newNode(ND_GOTO, Tok);
    Nd->Label = getIdent(Tok->Next);

    Nd->GotoNext = Gotos;
    Gotos = Nd;
    *Rest = skip(Tok->Next->Next, ";");
    return Nd;
  }


  // "break" ";"
  if (equal(Tok, "break")) {
    if (!BrkLabel){
      errorTok(Tok, "stray break");
    }

    Node *Nd = newNode(ND_GOTO, Tok);
    Nd->UniqueLabel = BrkLabel;
    *Rest = skip(Tok->Next, ";");
    return Nd;

  }

  // "continue" ";"
if(equal(Tok, "continue")){
  if(!ContLabel){
    errorTok(Tok, "stray continute");
  }

  Node *Nd = newNode(ND_GOTO, Tok);
  Nd->UniqueLabel = ContLabel;
  *Rest  = skip(Tok->Next, ";");
  return Nd;
}

  // ident ":" stmt
  if (Tok->Kind == TK_IDENT && equal(Tok->Next, ":")) {
    Node *Nd = newNode(ND_LABEL, Tok);
    Nd->Label = strndup(Tok->Loc, Tok->Len);
    Nd->UniqueLabel = newUniqueName();
    Nd->LHS = stmt(Rest, Tok->Next->Next);

    Nd->GotoNext = Labels;
    Labels = Nd;
    return Nd;
  }
  
  // "{" compoundStmt
  if (equal(Tok, "{"))
    return compoundStmt(Rest, Tok->Next);

  // exprStmt
  return exprStmt(Rest, Tok);
}

// compoundStmt = (typedef | declaration | stmt)* "}"
static Node *compoundStmt(Token **Rest, Token *Tok)
{

  Node Head = {};
  Node *Cur = &Head;

  // enter new scope
  enterScope();

  // (declaration | stmt)* "}"
  while (!equal(Tok, "}"))
  {
    // declaration
    if (isTypename(Tok) && !equal(Tok->Next, ":"))
    {
      VarAttr Attr = {};
      Type *BaseTy = declspec(&Tok, Tok, &Attr);

      // parse typedef statement
      if (Attr.IsTypedef)
      {
        Tok = parseTypedef(Tok, BaseTy);
        continue;
      }

      // parse variable statement
      Cur->Next = declaration(&Tok, Tok, BaseTy);
    }
    // stmt
    else
    {
      Cur->Next = stmt(&Tok, Tok);
    }
    Cur = Cur->Next;

    addType(Cur);
  }

  // end current scope
  leaveScope();

  // save {} block in Node's Body;
  Node *Nd = newNode(ND_BLOCK, Tok);
  Nd->Body = Head.Next;
  *Rest = Tok->Next;
  return Nd;
}

// exprStmt = expr? ";"
static Node *exprStmt(Token **Rest, Token *Tok)
{
  // ";"
  if (equal(Tok, ";"))
  {
    *Rest = Tok->Next;
    return newNode(ND_BLOCK, Tok);
  }

  // expr ";"
  Node *Nd = newNode(ND_EXPR_STMT, Tok);
  Nd->LHS = expr(&Tok, Tok);
  *Rest = skip(Tok, ";");
  return Nd;
}

// expr = assign ("," expr)?
static Node *expr(Token **Rest, Token *Tok)
{
  Node *Nd = assign(&Tok, Tok);

  if (equal(Tok, ","))
  {
    return newBinary(ND_COMMA, Nd, expr(Rest, Tok->Next), Tok);
  }

  *Rest = Tok;
  return Nd;
}

// 转换 A op= B为 TMP = &A, *TMP = *TMP op B
static Node *toAssign(Node *Binary) {
  // A
  addType(Binary->LHS);
  // B
  addType(Binary->RHS);
  Token *Tok = Binary->Tok;

  // TMP
  Obj *Var = newLVar("", pointerTo(Binary->LHS->Ty));

  // TMP = &A
  Node *Expr1 = newBinary(ND_ASSIGN, newVarNode(Var, Tok),
                          newUnary(ND_ADDR, Binary->LHS, Tok), Tok);

  // *TMP = *TMP op B
  Node *Expr2 = newBinary(
      ND_ASSIGN, newUnary(ND_DEREF, newVarNode(Var, Tok), Tok),
      newBinary(Binary->Kind, newUnary(ND_DEREF, newVarNode(Var, Tok), Tok),
                Binary->RHS, Tok),
      Tok);

  // TMP = &A, *TMP = *TMP op B
  return newBinary(ND_COMMA, Expr1, Expr2, Tok);
}

// assign = logOr (assignOp assign)?
// assignOp = "=" | "+=" | "-=" | "*=" | "/=" | "%=" | "&=" | "|=" | "^="
//          | "<<=" | ">>="
static Node *assign(Token **Rest, Token *Tok)
{
  // equality
  Node *Nd = logOr(&Tok, Tok);

  // 可能存在递归赋值，如a=b=1
  // ("=" assign)?
  if (equal(Tok, "=")){
        return Nd = newBinary(ND_ASSIGN, Nd, assign(Rest, Tok->Next), Tok);
  }
  
    // ("+=" assign)?
  if (equal(Tok, "+=")){
    return toAssign(newAdd(Nd, assign(Rest, Tok->Next), Tok));
  }

  // ("-=" assign)?
  if (equal(Tok, "-=")){
    return toAssign(newSub(Nd, assign(Rest, Tok->Next), Tok));
  }

  // ("*=" assign)?
  if (equal(Tok, "*=")){
    return toAssign(newBinary(ND_MUL, Nd, assign(Rest, Tok->Next), Tok));
  }

  // ("/=" assign)?
  if (equal(Tok, "/=")){
    return toAssign(newBinary(ND_DIV, Nd, assign(Rest, Tok->Next), Tok));
  }

  // ("%=" assign)?
  if (equal(Tok, "%=")){
    return toAssign(newBinary(ND_MOD, Nd, assign(Rest, Tok->Next), Tok));
  }

  // ("&=" assign)?
  if (equal(Tok, "&=")){
    return toAssign(newBinary(ND_BITAND, Nd, assign(Rest, Tok->Next), Tok));
  }

  // ("|=" assign)?
  if (equal(Tok, "|=")){
    return toAssign(newBinary(ND_BITOR, Nd, assign(Rest, Tok->Next), Tok));
  }

  // ("^=" assign)?
  if (equal(Tok, "^=")){
    return toAssign(newBinary(ND_BITXOR, Nd, assign(Rest, Tok->Next), Tok));
  }

  // ("<<=" assign)?
  if (equal(Tok, "<<=")){
    return toAssign(newBinary(ND_SHL, Nd, assign(Rest, Tok->Next), Tok));
  }

  // (">>=" assign)?
  if (equal(Tok, ">>=")){
    return toAssign(newBinary(ND_SHR, Nd, assign(Rest, Tok->Next), Tok));
  }    


  *Rest = Tok;
  return Nd;
}

// logOr = logAnd ("||" logAnd)*
static Node *logOr(Token **Rest, Token *Tok) {
  Node *Nd = logAnd(&Tok, Tok);
  while (equal(Tok, "||")) {
    Token *Start = Tok;
    Nd = newBinary(ND_LOGOR, Nd, logAnd(&Tok, Tok->Next), Start);
  }
  *Rest = Tok;
  return Nd;
}

// logAnd = bitOr ("&&" bitOr)*
static Node *logAnd(Token **Rest, Token *Tok) {
  Node *Nd = bitOr(&Tok, Tok);
  while (equal(Tok, "&&")) {
    Token *Start = Tok;
    Nd = newBinary(ND_LOGAND, Nd, bitOr(&Tok, Tok->Next), Start);
  }
  *Rest = Tok;
  return Nd;
}

// bitOr = bitXor ("|" bitXor)*
static Node *bitOr(Token **Rest, Token *Tok) {
  Node *Nd = bitXor(&Tok, Tok);
  while (equal(Tok, "|")) {
    Token *Start = Tok;
    Nd = newBinary(ND_BITOR, Nd, bitXor(&Tok, Tok->Next), Start);
  }
  *Rest = Tok;
  return Nd;
}

// bitXor = bitAnd ("^" bitAnd)*
static Node *bitXor(Token **Rest, Token *Tok) {
  Node *Nd = bitAnd(&Tok, Tok);
  while (equal(Tok, "^")) {
    Token *Start = Tok;
    Nd = newBinary(ND_BITXOR, Nd, bitAnd(&Tok, Tok->Next), Start);
  }
  *Rest = Tok;
  return Nd;
}

// bitAnd = equality ("&" equality)*
static Node *bitAnd(Token **Rest, Token *Tok) {
  Node *Nd = equality(&Tok, Tok);
  while (equal(Tok, "&")) {
    Token *Start = Tok;
    Nd = newBinary(ND_BITAND, Nd, equality(&Tok, Tok->Next), Start);
  }
  *Rest = Tok;
  return Nd;
}
// equality = relational ("==" relational | "!=" relational)*
static Node *equality(Token **Rest, Token *Tok)
{
  // relational
  Node *Nd = relational(&Tok, Tok);

  // ("==" relational | "!=" relational)*
  while (true)
  {
    Token *Start = Tok;
    // "==" relational
    if (equal(Tok, "=="))
    {
      Nd = newBinary(ND_EQ, Nd, relational(&Tok, Tok->Next), Start);
      continue;
    }

    // "!=" relational
    if (equal(Tok, "!="))
    {
      Nd = newBinary(ND_NE, Nd, relational(&Tok, Tok->Next), Start);
      continue;
    }

    *Rest = Tok;
    return Nd;
  }
}

// relational = shift ("<" shift | "<=" shift | ">" shift | ">=" shift)*
static Node *relational(Token **Rest, Token *Tok)
{
  // shift
  Node *Nd = shift(&Tok, Tok);

  // ("<" shift | "<=" shift | ">" shift | ">=" shift)*
  while (true)
  {
    Token *Start = Tok;

      // "<" shift
    if (equal(Tok, "<"))
    {
      Nd = newBinary(ND_LT, Nd, shift(&Tok, Tok->Next), Start);
      continue;
    }

    // "<=" shift
    if (equal(Tok, "<="))
    {
      Nd = newBinary(ND_LE, Nd, shift(&Tok, Tok->Next), Start);
      continue;
    }

    // ">" shift
    // X>Y  ~ Y<X
    if (equal(Tok, ">"))
    {
      Nd = newBinary(ND_LT, shift(&Tok, Tok->Next), Nd, Start);
      continue;
    }

    // ">=" shift
    // X>=Y ~ Y<=X
    if (equal(Tok, ">="))
    {
      Nd = newBinary(ND_LE, shift(&Tok, Tok->Next), Nd, Start);
      continue;
    }

    *Rest = Tok;
    return Nd;
  }
}


// shift = add ("<<" add | ">>" add)*
static Node *shift(Token **Rest, Token *Tok) {
  // add
  Node *Nd = add(&Tok, Tok);

  while (true) {
    Token *Start = Tok;

    // "<<" add
    if (equal(Tok, "<<")) {
      Nd = newBinary(ND_SHL, Nd, add(&Tok, Tok->Next), Start);
      continue;
    }

    // ">>" add
    if (equal(Tok, ">>")) {
      Nd = newBinary(ND_SHR, Nd, add(&Tok, Tok->Next), Start);
      continue;
    }

    *Rest = Tok;
    return Nd;
  }
}
// handle `add`
static Node *newAdd(Node *LHS, Node *RHS, Token *Tok)
{
  addType(LHS);
  addType(RHS);

  // num + num
  if (isInteger(LHS->Ty) && isInteger(RHS->Ty))
  {
    return newBinary(ND_ADD, LHS, RHS, Tok);
  }

  // could not handle pointer + pointer
  if (LHS->Ty->Base && RHS->Ty->Base)
  {
    errorTok(Tok, "invalid operands");
  }

  // 前面已经排除了两个指针的情况
  // 如果第一个表达式成立，则进行互换 num+ptr -> ptr + num
  if (!LHS->Ty->Base && RHS->Ty->Base)
  {
    Node *Tmp = LHS;
    LHS = RHS;
    RHS = Tmp;
  }

  // ptr + 1
  // 1 ： size of one element
  // need x size
  // use long type to save pointer
  RHS = newBinary(ND_MUL, RHS, newLong(LHS->Ty->Base->Size, Tok), Tok);
  return newBinary(ND_ADD, LHS, RHS, Tok);
}

// handle  `sub`
static Node *newSub(Node *LHS, Node *RHS, Token *Tok)
{

  addType(LHS);
  addType(RHS);

  // num - num
  if (isInteger(LHS->Ty) && isInteger(RHS->Ty))
  {
    return newBinary(ND_SUB, LHS, RHS, Tok);
  }

  // ptr - num
  if (LHS->Ty->Base && isInteger(RHS->Ty))
  {
    // use long type to save pointer
    RHS = newBinary(ND_MUL, RHS, newLong(LHS->Ty->Base->Size, Tok), Tok);
    addType(RHS);
    Node *Nd = newBinary(ND_SUB, LHS, RHS, Tok);

    // node type is pointer
    Nd->Ty = LHS->Ty;
    return Nd;
  }

  // ptr - ptr，return the num of element between two pointer
  if (LHS->Ty->Base && RHS->Ty->Base)
  {
    Node *Nd = newBinary(ND_SUB, LHS, RHS, Tok);
    Nd->Ty = TyInt;
    return newBinary(ND_DIV, Nd, newNum(LHS->Ty->Base->Size, Tok), Tok);
  }

  errorTok(Tok, "invalid operands");
  return NULL;
}

// add = mul ("+" mul | "-" mul)*
static Node *add(Token **Rest, Token *Tok)
{
  // mul
  Node *Nd = mul(&Tok, Tok);

  // ("+" mul | "-" mul)*
  while (true)
  {
    Token *Start = Tok;

    // "+" mul
    if (equal(Tok, "+"))
    {
      Nd = newAdd(Nd, mul(&Tok, Tok->Next), Start);
      continue;
    }

    // "-" mul
    if (equal(Tok, "-"))
    {
      Nd = newSub(Nd, mul(&Tok, Tok->Next), Start);
      continue;
    }

    *Rest = Tok;
    return Nd;
  }
}

// 解析乘除
// mul = cast ("*" cast | "/" cast | "%" cast)*
static Node *mul(Token **Rest, Token *Tok)
{
  // cast
  Node *Nd = cast(&Tok, Tok);

  // ("*" cast | "/" cast | "%" cast)*
  while (true)
  {

    Token *Start = Tok;

    // "*" cast
    if (equal(Tok, "*"))
    {
      Nd = newBinary(ND_MUL, Nd, cast(&Tok, Tok->Next), Start);
      continue;
    }

    // "/" cast
    if (equal(Tok, "/"))
    {
      Nd = newBinary(ND_DIV, Nd, cast(&Tok, Tok->Next), Start);
      continue;
    }

    // "%" cast
    if (equal(Tok, "%")) {
      Nd = newBinary(ND_MOD, Nd, cast(&Tok, Tok->Next), Start);
      continue;
    }

    *Rest = Tok;
    return Nd;
  }
}

// parse cast
// cast = "(" typeName ")" cast | unary
static Node *cast(Token **Rest, Token *Tok)
{
  // cast = "(" typeName ")" cast
  if (equal(Tok, "(") && isTypename(Tok->Next))
  {
    Token *Start = Tok;
    Type *Ty = typename(&Tok, Tok->Next);
    Tok = skip(Tok, ")");
    Node *Nd = newCast(cast(Rest, Tok), Ty);
    Nd->Tok = Start;
    return Nd;
  }

  // unary
  return unary(Rest, Tok);
}

// unary = ("+" | "-" | "*" | "&" | "!" | "~") cast
//       | ("++" | "--") unary
//       | postfix
static Node *unary(Token **Rest, Token *Tok)
{
  // "+" cast
  if (equal(Tok, "+")){
    return cast(Rest, Tok->Next);
  }

  // "-" cast
  if (equal(Tok, "-")){
    return newUnary(ND_NEG, cast(Rest, Tok->Next), Tok);
  }

  // "&" cast
  if (equal(Tok, "&")){
    return newUnary(ND_ADDR, cast(Rest, Tok->Next), Tok);
  }

  // "*" cast
  if (equal(Tok, "*")){
    return newUnary(ND_DEREF, cast(Rest, Tok->Next), Tok);
  }

  // "!" cast
  if (equal(Tok, "!")){
    return newUnary(ND_NOT, cast(Rest, Tok->Next), Tok);
  }

  // "~" cast
  if (equal(Tok, "~")){
    return newUnary(ND_BITNOT, cast(Rest, Tok->Next), Tok);
  }

  // ++i ->  i+=1
  // "++" unary
  if (equal(Tok, "++")){
    return toAssign(newAdd(unary(Rest, Tok->Next), newNum(1, Tok), Tok));
  }

  // +-i ->  i-=1
  // "--" unary
  if (equal(Tok, "--")){
    return toAssign(newSub(unary(Rest, Tok->Next), newNum(1, Tok), Tok));
  }

  // postfix
  return postfix(Rest, Tok);
}

// structMembers = (declspec declarator (","  declarator)* ";")*
static void structMembers(Token **Rest, Token *Tok, Type *Ty)
{
  Member Head = {};
  Member *Cur = &Head;

  while (!equal(Tok, "}"))
  {
    // declspec
    Type *BaseTy = declspec(&Tok, Tok, NULL);
    int First = true;

    while (!consume(&Tok, Tok, ";"))
    {
      if (!First)
      {
        Tok = skip(Tok, ",");
      }
      First = false;

      Member *Mem = calloc(1, sizeof(Member));

      // declarator
      Mem->Ty = declarator(&Tok, Tok, BaseTy);
      Mem->Name = Mem->Ty->Name;
      Cur = Cur->Next = Mem;
    }
  }

  *Rest = Tok->Next;
  Ty->Mems = Head.Next;
}

// structUnionDecl = ident? ("{" structMembers)?
static Type *structUnionDecl(Token **Rest, Token *Tok)
{

  // read struct tag
  Token *Tag = NULL;
  if (Tok->Kind == TK_IDENT)
  {
    Tag = Tok;
    Tok = Tok->Next;
  }

  if (Tag && !equal(Tok, "{"))
  {
    *Rest = Tok;

    Type *Ty = findTag(Tag);
    if (Ty){
      return Ty;
    }

    Ty = structType();
    Ty->Size = -1;
    pushTagScope(Tag, Ty);
    return Ty;
  }

  // ("{" structMembers)?
  Tok = skip(Tok, "{");

  // create a new struct
  Type *Ty = structType();
  structMembers(Rest, Tok, Ty);

  Ty->Align = 1;

  // cover before define
  if (Tag) {
    for (TagScope *S = Scp->Tags; S; S = S->Next) {
      if (equal(Tag, S->Name)) {
        *S->Ty = *Ty;
        return S->Ty;
      }
    }

    pushTagScope(Tag, Ty);
  }

  return Ty;
}

// structDecl = structUnionDecl
static Type *structDecl(Token **Rest, Token *Tok)
{
  Type *Ty = structUnionDecl(Rest, Tok);
  Ty->Kind = TY_STRUCT;

  // caculate struct emember's offset
  int Offset = 0;
  for (Member *Mem = Ty->Mems; Mem; Mem = Mem->Next)
  {
    Offset = alignTo(Offset, Mem->Ty->Align);
    Mem->Offset = Offset;
    Offset += Mem->Ty->Size;

    if (Ty->Align < Mem->Ty->Align)
    {
      Ty->Align = Mem->Ty->Align;
    }
  }
  Ty->Size = alignTo(Offset, Ty->Align);

  return Ty;
}

// unionDecl = structUnionDecl
static Type *unionDecl(Token **Rest, Token *Tok)
{
  Type *Ty = structUnionDecl(Rest, Tok);
  Ty->Kind = TY_UNION;

  for (Member *Mem = Ty->Mems; Mem; Mem = Mem->Next)
  {
    if (Ty->Align < Mem->Ty->Align)
    {
      Ty->Align = Mem->Ty->Align;
    }
    if (Ty->Size < Mem->Ty->Size)
    {
      Ty->Size = Mem->Ty->Size;
    }
  }

  Ty->Size = alignTo(Ty->Size, Ty->Align);
  return Ty;
}

// get struct member
static Member *getStructMember(Type *Ty, Token *Tok)
{
  for (Member *Mem = Ty->Mems; Mem; Mem = Mem->Next)
    if (Mem->Name->Len == Tok->Len &&
        !strncmp(Mem->Name->Loc, Tok->Loc, Tok->Len))
      return Mem;
  errorTok(Tok, "no such member");
  return NULL;
}

// build struct member node
static Node *structRef(Node *LHS, Token *Tok)
{
  addType(LHS);
  if (LHS->Ty->Kind != TY_STRUCT && LHS->Ty->Kind != TY_UNION)
  {
    errorTok(LHS->Tok, "not a struct nor a union");
  }

  Node *Nd = newUnary(ND_MEMBER, LHS, Tok);
  Nd->Mem = getStructMember(LHS->Ty, Tok);
  return Nd;
}

// A++ to `(typeof A)((A += 1) - 1)`
// Increase Decrease
static Node *newIncDec(Node *Nd, Token *Tok, int Addend) {
  addType(Nd);
  return newCast(newAdd(toAssign(newAdd(Nd, newNum(Addend, Tok), Tok)),
                        newNum(-Addend, Tok), Tok),
                 Nd->Ty);
}


// postfix = primary ("[" expr "]" | "." ident)* | "->" ident | "++" | "--")*
static Node *postfix(Token **Rest, Token *Tok)
{
  // primary
  Node *Nd = primary(&Tok, Tok);

  // ("[" expr "]")*
  while (true)
  {
    if (equal(Tok, "["))
    {
      // x[y] 等价于 *(x+y)
      Token *Start = Tok;
      Node *Idx = expr(&Tok, Tok->Next);
      Tok = skip(Tok, "]");
      Nd = newUnary(ND_DEREF, newAdd(Nd, Idx, Start), Start);
      continue;
    }

    // "." ident
    if (equal(Tok, "."))
    {
      Nd = structRef(Nd, Tok->Next);
      Tok = Tok->Next->Next;
      continue;
    }

    // "->" ident
    if (equal(Tok, "->"))
    {
      // x -> y  ==  (*x).y
      Nd = newUnary(ND_DEREF, Nd, Tok);
      Nd = structRef(Nd, Tok->Next);
      Tok = Tok->Next->Next;
      continue;
    }

    if (equal(Tok, "++")) {
      Nd = newIncDec(Nd, Tok, 1);
      Tok = Tok->Next;
      continue;
    }

    if (equal(Tok, "--")) {
      Nd = newIncDec(Nd, Tok, -1);
      Tok = Tok->Next;
      continue;
    }
    
    *Rest = Tok;
    return Nd;
  }
}

// 解析函数调用
// funcall = ident "(" (assign ("," assign)*)? ")"
static Node *funCall(Token **Rest, Token *Tok)
{
  Token *Start = Tok;
  Tok = Tok->Next->Next;

  // find function name
  VarScope *S = findVar(Start);
  if (!S)
  {
    errorTok(Start, "implicit declaration of a function");
  }
  if (!S->Var || S->Var->Ty->Kind != TY_FUNC)
  {
    errorTok(Start, "not a function");
  }

  // 函数名类型
  Type *Ty = S->Var->Ty;

  // 函数形参的类型
  Type *ParamTy = Ty->Params;

  Node Head = {};
  Node *Cur = &Head;

  while (!equal(Tok, ")"))
  {
    if (Cur != &Head)
    {
      Tok = skip(Tok, ",");
    }
    // assign
    Node *Arg = assign(&Tok, Tok);
    addType(Arg);

    if (ParamTy)
    {
      if (ParamTy->Kind == TY_STRUCT || ParamTy->Kind == TY_UNION)
      {
        errorTok(Arg->Tok, "passing struct or union is not supported yet");
      }

      // change param type
      Arg = newCast(Arg, ParamTy);
      ParamTy = ParamTy->Next;
    }
    Cur->Next = Arg;
    Cur = Cur->Next;
    addType(Cur);
  }

  *Rest = skip(Tok, ")");

  Node *Nd = newNode(ND_FUNCALL, Start);
  // ident
  Nd->FuncName = strndup(Start->Loc, Start->Len);

  // function type
  Nd->FuncType = Ty;
  // read return value
  Nd->Ty = Ty->ReturnTy;
  Nd->Args = Head.Next;
  return Nd;
}

// 解析括号、数字、变量
// primary = "(" "{" stmt+ "}" ")"
//         | "(" expr ")"
//         | "sizeof" "(" typeName ")"
//         | "sizeof" unary
//         | ident funcArgs?
//         | str
//         | num
static Node *primary(Token **Rest, Token *Tok)
{
  Token *Start = Tok;

  // "(" "{" stmt + "}" ")"
  if (equal(Tok, "(") && equal(Tok->Next, "{"))
  {

    Node *Nd = newNode(ND_STMT_EXPR, Tok);
    Nd->Body = compoundStmt(&Tok, Tok->Next->Next)->Body;
    *Rest = skip(Tok, ")");
    return Nd;
  }
  // "(" expr ")"
  if (equal(Tok, "("))
  {
    Node *Nd = expr(&Tok, Tok->Next);
    *Rest = skip(Tok, ")");
    return Nd;
  }

  // "sizeof" "(" typeName ")"
  if (equal(Tok, "sizeof") && equal(Tok->Next, "(") &&
      isTypename(Tok->Next->Next))
  {
    Type *Ty = typename(&Tok, Tok->Next->Next);
    *Rest = skip(Tok, ")");
    return newNum(Ty->Size, Start);
  }

  // "sizeof" unary
  if (equal(Tok, "sizeof"))
  {
    Node *Nd = unary(Rest, Tok->Next);
    addType(Nd);
    return newNum(Nd->Ty->Size, Tok);
  }

  // ident args?
  if (Tok->Kind == TK_IDENT)
  {
    // function call
    // args =  "("")"
    if (equal(Tok->Next, "("))
    {
      return funCall(Rest, Tok);
    }

    // ident
    // find variable and enum from locals
    VarScope *S = findVar(Tok);
    // if no exist before, create one
    if (!S || (!S->Var && !S->EnumTy))
    {
      errorTok(Tok, "undefined variable");
    }

    Node *Nd;
    
    if (S->Var)
    {
      Nd = newVarNode(S->Var, Tok);
    }
    else
    {
      Nd = newNum(S->EnumVal, Tok);
    }

    *Rest = Tok->Next;
    return Nd;
  }

  // str
  if (Tok->Kind == TK_STR)
  {
    Obj *Var = newStringLiteral(Tok->Str, Tok->Ty);
    *Rest = Tok->Next;
    return newVarNode(Var, Tok);
  }

  // num
  if (Tok->Kind == TK_NUM)
  {
    Node *Nd = newNum(Tok->Val, Tok);
    *Rest = Tok->Next;
    return Nd;
  }

  errorTok(Tok, "expected an expression");
  return NULL;
}

// parse type alias
static Token *parseTypedef(Token *Tok, Type *BaseTy)
{
  bool First = true;

  while (!consume(&Tok, Tok, ";"))
  {
    if (!First)
    {
      Tok = skip(Tok, ",");
    }
    First = false;

    Type *Ty = declarator(&Tok, Tok, BaseTy);
    // save type alias into variable scope and set type;
    pushScope(getIdent(Ty->Name))->Typedef = Ty;
  }
  return Tok;
}

// add params to the Locals
// reverse ?
static void createParamLVars(Type *Param)
{
  if (Param)
  {
    createParamLVars(Param->Next);
    // add to local
    newLVar(getIdent(Param->Name), Param);
  }
}

static void resolveGotoLabels(void) {
  for (Node *X = Gotos; X; X = X->GotoNext) {
    for (Node *Y = Labels; Y; Y = Y->GotoNext) {
      if (!strcmp(X->Label, Y->Label)) {
        X->UniqueLabel = Y->UniqueLabel;
        break;
      }
    }

    if (X->UniqueLabel == NULL)
      errorTok(X->Tok->Next, "use of undeclared label");
  }

  Gotos = NULL;
  Labels = NULL;
}

// functionDefinition = declspec declarator? ident "(" ")" "{" compoundStmt*
static Token *function(Token *Tok, Type *BaseTy, VarAttr *Attr)
{
  Type *Ty = declarator(&Tok, Tok, BaseTy);

  Obj *Fn = newGVar(getIdent(Ty->Name), Ty);
  Fn->IsFunction = true;

  Fn->IsDefinition = !consume(&Tok, Tok, ";");
  Fn->IsStatic = Attr->IsStatic;

  if (!Fn->IsDefinition)
  {
    return Tok;
  }

  CurrentFn = Fn;

  // clear locals
  Locals = NULL;

  enterScope();

  // handle params
  createParamLVars(Ty->Params);
  Fn->Params = Locals;

  Tok = skip(Tok, "{");
  Fn->Body = compoundStmt(&Tok, Tok);
  Fn->Locals = Locals;

  leaveScope();

  resolveGotoLabels();
  return Tok;
}

// build global variable
static Token *globalVariable(Token *Tok, Type *Basety)
{
  bool First = true;

  while (!consume(&Tok, Tok, ";"))
  {
    if (!First)
    {
      Tok = skip(Tok, ",");
    }
    First = false;

    Type *Ty = declarator(&Tok, Tok, Basety);
    newGVar(getIdent(Ty->Name), Ty);
  }

  return Tok;
}

// judge function or global variable
static bool isFunction(Token *Tok)
{
  if (equal(Tok, ";"))
  {
    return false;
  }

  Type Dummy = {};
  Type *Ty = declarator(&Tok, Tok, &Dummy);
  return Ty->Kind == TY_FUNC;
}

// syntax parser entry function
// program = (typedef | functionDefinition* | global-variable)*
Obj *parse(Token *Tok)
{
  Globals = NULL;

  while (Tok->Kind != TK_EOF)
  {
    VarAttr Attr = {};
    Type *BaseTy = declspec(&Tok, Tok, &Attr);

    // typedef
    if (Attr.IsTypedef)
    {
      Tok = parseTypedef(Tok, BaseTy);
      continue;
    }
    // function
    if (isFunction(Tok))
    {
      Tok = function(Tok, BaseTy, &Attr);
      continue;
    }

    // global variable
    Tok = globalVariable(Tok, BaseTy);
  }

  return Globals;
}