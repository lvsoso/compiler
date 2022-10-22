#include "rvcc.h"

// local or global variable scope
typedef struct VarScope VarScope;
struct VarScope
{
  VarScope *Next;
  char *Name;
  Obj *Var;
};

// struct tag and union tag scope
typedef struct TagScope TagScope;
struct TagScope {
  TagScope *Next; // next tag scope
  char *Name; // scope name
  Type *Ty;   // scope type
};



typedef struct Scope Scope;
struct Scope {
  Scope *Next;
  
  // C有两个域：变量域，结构体标签域
  VarScope *Vars; // 指向当前域内的变量
  TagScope *Tags; // 指向当前域内的结构体标签

};


// all scope link
static Scope *Scp = &(Scope){};

// save local variables
Obj *Locals;
// save global variables
Obj *Globals; 

/// Some "*" just for the pointer

// program = (functionDefinition* | global-variable)*
// functionDefinition = declspec declarator? ident "(" ")" "{" compoundStmt*
// declspec = "char" | "short" | "int" | "long" | structDecl | unionDecl
// declarator = "*"* ("(" ident ")" | "(" declarator ")" | ident) typeSuffix
// typeSuffix = "(" funcParams | "[" num "]" typeSuffix | ε
// funcParams = (param ("," param)*)? ")"
// param = declspec declarator
// compoundStmt =  (declaration | stmt)* "}"
// declaration =
//    declspec (declarator ("=" expr)? ("," declarator ("=" expr)?)*)? ";"
// stmt = "return" expr ";"
//        | "if" "(" expr ")" stmt ("else" stmt )?
//        | "for" "(" exprStmt expr? ";" expr? ")" stmt
//        | "while" "(" expr ")" stmt
//        | "{" compoundStmt
//        | exprStmt
// exprStmt = expr? ";"
// expr = assign ("," expr)?
// assign = equality ("=" assign)?
// equality = relational ("==" relational | "!=" relational)*
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
// add = mul ("+" mul | "-" mul)*
// mul = unary ("*" unary | "/" unary)*
// unary = ("+" | "-" | "*" | "&") unary | postfix
// structMembers = (declspec declarator (","  declarator)* ";")*
// structDecl = structUnionDecl
// unionDecl = structUnionDecl
// structUnionDecl = ident? ("{" structMembers)?
// postfix = primary ("[" expr "]" | "." ident)* | "->" ident)*
// primary =  "(" "{" stmt+ "}" ")"
//         | "(" expr ")"
//         | "sizeof" unary
//         | ident funcArgs?
//         | str
//         | num

// funcall = ident "(" (assign ("," assign)*)? ")"
static Type *declspec(Token **Rest, Token *Tok);
static Type *declarator(Token **Rest, Token *Tok, Type *Ty);
static Node *declaration(Token **Rest, Token *Tok);
static Node *compoundStmt(Token **Rest, Token *tok);
static Node *stmt(Token **Rest, Token *Tok);
static Node *exprStmt(Token **Rest, Token *Tok);
static Node *expr(Token **Rest, Token *Tok);
static Node *assign(Token **Rest, Token *Tok);
static Node *equality(Token **Rest, Token *Tok);
static Node *relational(Token **Rest, Token *Tok);
static Node *add(Token **Rest, Token *Tok);
static Node *mul(Token **Rest, Token *Tok);
static Type *structDecl(Token **Rest, Token *Tok);
static Type *unionDecl(Token **Rest, Token *Tok);
static Node *unary(Token **Rest, Token *Tok);
static Node *postfix(Token **Rest, Token *Tok);
static Node *primary(Token **Rest, Token *Tok);


// Enter scope
static void enterScope(void){
  Scope *S = calloc(1, sizeof(Scope));
  S->Next = Scp;
  Scp = S;
}


//  exit scope
static void leaveScope(void){Scp = Scp->Next;}

// find local variable by name
static Obj *findVar(Token *Tok)
{
  // find from scope first, deepin first
  for (Scope *S = Scp; S; S = S->Next)
  {
    // for-loop all variable in scope
    for(VarScope *S2 = S->Vars; S2; S2 = S2 -> Next)
    {
      if(equal(Tok, S2->Name))
      {
        return S2->Var;
      }
    }
  }
  return NULL;
}

// find tag by token
static Type *findTag(Token *Tok) {
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

// new variable
static Node *newVarNode(Obj *Var, Token *Tok)
{
  Node *Nd = newNode(ND_VAR, Tok);
  Nd->Var = Var;
  return Nd;
}

// save variable in current scope
static VarScope *pushScope(char *Name, Obj *Var)
{
  VarScope *S = calloc(1, sizeof(VarScope));
  S->Name = Name;
  S->Var = Var;

  // append to the head of linker;
  S->Next = Scp -> Vars;
  Scp -> Vars = S;
  return S;
}

// new variable
static Obj *newVar(char *Name, Type *Ty) 
{
  Obj *Var = calloc(1, sizeof(Obj));
  Var -> Name = Name;
  Var -> Ty = Ty;
  pushScope(Name, Var);
  return Var;
}

// insert new variable to 'locals'
static Obj *newLVar(char *Name, Type *Ty)
{
  Obj* Var = newVar(Name, Ty);
  Var->IsLocal = true;
  // the new one be the head
  Var->Next = Locals;
  Locals = Var;
  return Var;
}

// add new global variable
static Obj *newGVar(char *Name, Type *Ty) 
{
  Obj* Var = newVar(Name, Ty);
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
static Obj *newStringLiteral(char *Str, Type *Ty) {
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

// get number
static int64_t getNumber(Token *Tok){
  if(Tok->Kind != TK_NUM){
    errorTok(Tok, "expected a number");
  }
  return Tok->Val;
}

static void pushTagScope(Token *Tok, Type *Ty){
  TagScope *S = calloc(1, sizeof(TagScope));
  S->Name = strndup(Tok->Loc, Tok->Len);
  S->Ty = Ty;
  S->Next = Scp->Tags;
  Scp->Tags = S;
}

// declspec = "char" | "short" | "int"  | "long" | structDecl | unionDecl
// declarator specifier
static Type *declspec(Token **Rest, Token *Tok)
{
  // "char"
  if (equal(Tok, "char")) {
    *Rest = Tok->Next;
    return TyChar;
  }

  // "int"
  if (equal(Tok, "int")) {
    *Rest = Tok->Next;
    return TyInt;
  }

  // "short"
  if (equal(Tok, "short")) {
    *Rest = Tok->Next;
    return TyShort;
  }


  // "long"
  if (equal(Tok, "long")) {
    *Rest = Tok->Next;
    return TyLong;
  }


  // "struct"
  if(equal(Tok, "struct")){
    return structDecl(Rest, Tok->Next);
  }

  // "union"
  if (equal(Tok, "union")){
    return unionDecl(Rest, Tok->Next);
  }
  
  errorTok(Tok, "typename expected");
  return NULL;
}
// funcParams = (param ("," param)*)? ")"
// param = declspec declarator
static Type *funcParams(Token **Rest, Token *Tok, Type *Ty) {
  Type Head = {};
  Type *Cur = &Head;

  while (!equal(Tok, ")")) {
    // funcParams = param ("," param)*
    // param = declspec declarator
    if (Cur != &Head)
      Tok = skip(Tok, ",");
    Type *BaseTy = declspec(&Tok, Tok);
    Type *DeclarTy = declarator(&Tok, Tok, BaseTy);
    // copy the type
    Cur->Next = copyType(DeclarTy);
    Cur = Cur->Next;
  }

  // package a func type
  Ty = funcType(Ty);
  // pass the args
  Ty->Params = Head.Next;
  *Rest = Tok->Next;
  return Ty;
}

// typeSuffix = ("(" funcParams? ")")?
static Type *typeSuffix(Token **Rest, Token *Tok, Type *Ty) {
  if (equal(Tok, "("))
    return funcParams(Rest, Tok->Next, Ty);

  if (equal(Tok, "[")) {
    int Sz = getNumber(Tok->Next);
    Tok = skip(Tok->Next->Next, "]");
    Ty = typeSuffix(Rest, Tok, Ty);
    return arrayOf(Ty, Sz);
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
  if (equal(Tok, "(")) {
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

// declaration =
// declspec (declarator ("=" expr) ? ("," declarator("=" expr)?)*)? ";"
static Node *declaration(Token **Rest, Token *Tok)
{
  // declspec
  // declear base type
  Type *Basety = declspec(&Tok, Tok);

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
    Type *Ty = declarator(&Tok, Tok, Basety);
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
static bool isTypename(Token *Tok) {
  return equal(Tok, "char") || equal(Tok, "short") || equal(Tok, "int") || equal(Tok, "long") ||
         equal(Tok, "struct") || equal(Tok, "union");
}

// stmt = "return" expr ";"
//        | "if" "(" expr ")" stmt ("else" stmt )?
//        | "for" "(" exprStmt expr? ";" expr? ")" stmt
//        | "while" "(" expr ")" stmt
//        | "{" compoundStmt
//        | exprStmt
static Node *stmt(Token **Rest, Token *Tok)
{
  // "return" expr ";"
  if (equal(Tok, "return"))
  {
    Node *Nd = newNode(ND_RETURN, Tok);
    Nd->LHS = expr(&Tok, Tok->Next);
    *Rest = skip(Tok, ";");
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
    if (equal(Tok, "else"))
      Nd->Els = stmt(&Tok, Tok->Next);
    *Rest = Tok;
    return Nd;
  }

  // "for" "(" exprStmt expr? ";" expr? ")" stmt
  if (equal(Tok, "for"))
  {
    Node *Nd = newNode(ND_FOR, Tok);
    // "("
    Tok = skip(Tok->Next, "(");

    // exprStmt
    Nd->Init = exprStmt(&Tok, Tok);

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
    // stmt
    Nd->Then = stmt(Rest, Tok);
    return Nd;
  }

  // "{" compoundStmt
  if (equal(Tok, "{"))
    return compoundStmt(Rest, Tok->Next);

  // exprStmt
  return exprStmt(Rest, Tok);
}

// compoundStmt = (declaration | stmt)* "}"
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
   if (isTypename(Tok))
    {
      Cur->Next = declaration(&Tok, Tok);
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

  if (equal(Tok, ",")){
    return newBinary(ND_COMMA, Nd, expr(Rest, Tok->Next), Tok);
  }
  
  *Rest = Tok;
  return Nd;
}

// assign = equality ("=" assign)?
static Node *assign(Token **Rest, Token *Tok)
{
  // equality
  Node *Nd = equality(&Tok, Tok);

  // 可能存在递归赋值，如a=b=1
  // ("=" assign)?
  if (equal(Tok, "="))
    return Nd = newBinary(ND_ASSIGN, Nd, assign(Rest, Tok->Next), Tok);
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

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node *relational(Token **Rest, Token *Tok)
{
  // add
  Node *Nd = add(&Tok, Tok);

  // ("<" add | "<=" add | ">" add | ">=" add)*
  while (true)
  {
    Token *Start = Tok;

    // "<" add
    if (equal(Tok, "<"))
    {
      Nd = newBinary(ND_LT, Nd, add(&Tok, Tok->Next), Start);
      continue;
    }

    // "<=" add
    if (equal(Tok, "<="))
    {
      Nd = newBinary(ND_LE, Nd, add(&Tok, Tok->Next), Start);
      continue;
    }

    // ">" add
    // X>Y  ~ Y<X
    if (equal(Tok, ">"))
    {
      Nd = newBinary(ND_LT, add(&Tok, Tok->Next), Nd, Start);
      continue;
    }

    // ">=" add
    // X>=Y ~ Y<=X
    if (equal(Tok, ">="))
    {
      Nd = newBinary(ND_LE, add(&Tok, Tok->Next), Nd, Start);
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
  RHS = newBinary(ND_MUL, RHS,newNum(LHS->Ty->Base->Size, Tok), Tok);
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
    RHS = newBinary(ND_MUL, RHS, newNum(LHS->Ty->Base->Size, Tok), Tok);
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
// mul = unary ("*" unary | "/" unary)*
static Node *mul(Token **Rest, Token *Tok)
{
  Token *Start = Tok;

  // unary
  Node *Nd = unary(&Tok, Tok);

  // ("*" unary | "/" unary)*
  while (true)
  {
    // "*" unary
    if (equal(Tok, "*"))
    {
      Nd = newBinary(ND_MUL, Nd, unary(&Tok, Tok->Next), Start);
      continue;
    }

    // "/" unary
    if (equal(Tok, "/"))
    {
      Nd = newBinary(ND_DIV, Nd, unary(&Tok, Tok->Next), Start);
      continue;
    }

    *Rest = Tok;
    return Nd;
  }
}

// unary = ("+" | "-"  | "*" | "&") unary | postfix
static Node *unary(Token **Rest, Token *Tok)
{
  // "+" unary
  if (equal(Tok, "+"))
    return unary(Rest, Tok->Next);

  // "-" unary
  if (equal(Tok, "-"))
    return newUnary(ND_NEG, unary(Rest, Tok->Next), Tok);

  // "&" unary
  if (equal(Tok, "&"))
    return newUnary(ND_ADDR, unary(Rest, Tok->Next), Tok);

  // "*" unary
  if (equal(Tok, "*"))
    return newUnary(ND_DEREF, unary(Rest, Tok->Next), Tok);

  // postfix
  return postfix(Rest, Tok);
}

// structMembers = (declspec declarator (","  declarator)* ";")*
static void structMembers(Token **Rest, Token *Tok,  Type *Ty) {
  Member Head = {};
  Member *Cur = &Head;

  while (!equal(Tok, "}")){
    // declspec
    Type *BaseTy = declspec(&Tok, Tok);
    int First = true;

    while (!consume(&Tok, Tok, ";")) {
       if (!First)
      {
          Tok = skip(Tok, ",");
        }
        First = false;

        Member *Mem = calloc(1, sizeof(Member));

        // declarator
        Mem -> Ty = declarator(&Tok, Tok, BaseTy);
        Mem->Name = Mem->Ty->Name;
        Cur = Cur->Next = Mem;
    }
  }

  *Rest = Tok->Next;
  Ty->Mems = Head.Next;
}

// structUnionDecl = ident? ("{" structMembers)?
static Type *structUnionDecl(Token **Rest, Token *Tok) {

  // read struct tag
  Token *Tag = NULL;
  if (Tok->Kind == TK_IDENT) {
    Tag = Tok;
    Tok = Tok->Next;
  }

  if (Tag && !equal(Tok, "{")) {
    Type *Ty = findTag(Tag);
    if (!Ty){
        errorTok(Tag, "unknown struct type");
    }
    *Rest = Tok;
    return Ty;
  }

  // calloc a struct
  Type *Ty = calloc(1, sizeof(Type));
  Ty->Kind = TY_STRUCT;
  structMembers(Rest, Tok->Next, Ty);
  Ty->Align = 1;

  if (Tag)
    pushTagScope(Tag, Ty);
  return Ty;
}

// structDecl = structUnionDecl
static Type *structDecl(Token **Rest, Token *Tok) {
  Type *Ty = structUnionDecl(Rest, Tok);
  Ty->Kind = TY_STRUCT;

  // caculate struct emember's offset
  int Offset = 0;
  for (Member *Mem = Ty->Mems; Mem; Mem = Mem->Next) {
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
static Type *unionDecl(Token **Rest, Token *Tok) {
  Type *Ty = structUnionDecl(Rest, Tok);
  Ty->Kind = TY_UNION;

  for (Member *Mem = Ty->Mems; Mem; Mem = Mem->Next) {
    if (Ty->Align < Mem->Ty->Align){
      Ty->Align = Mem->Ty->Align;
    }
    if (Ty->Size < Mem->Ty->Size){
      Ty->Size = Mem->Ty->Size;
    }
  }
  
  Ty->Size = alignTo(Ty->Size, Ty->Align);
  return Ty;
}

// get struct member
static Member *getStructMember(Type *Ty, Token *Tok) {
  for (Member *Mem = Ty->Mems; Mem; Mem = Mem->Next)
    if (Mem->Name->Len == Tok->Len &&
        !strncmp(Mem->Name->Loc, Tok->Loc, Tok->Len))
      return Mem;
  errorTok(Tok, "no such member");
  return NULL;
}

//build struct member node
static Node *structRef(Node *LHS, Token *Tok) {
  addType(LHS);
if (LHS->Ty->Kind != TY_STRUCT && LHS->Ty->Kind != TY_UNION){
  errorTok(LHS->Tok, "not a struct nor a union");
}

  Node *Nd = newUnary(ND_MEMBER, LHS, Tok);
  Nd->Mem = getStructMember(LHS->Ty, Tok);
  return Nd;
}

// postfix = primary ("[" expr "]" | "." ident)* | "->" ident)*
static  Node* postfix(Token **Rest, Token *Tok)
{
  // primary
  Node *Nd = primary(&Tok, Tok);

  // ("[" expr "]")*
  while (true) {
    if (equal(Tok, "[")) {
      // x[y] 等价于 *(x+y)
      Token *Start = Tok;
      Node *Idx = expr(&Tok, Tok->Next);
      Tok = skip(Tok, "]");
      Nd = newUnary(ND_DEREF, newAdd(Nd, Idx, Start), Start);
      continue;
    }

    // "." ident
    if (equal(Tok, ".")) {
      Nd = structRef(Nd, Tok->Next);
      Tok = Tok->Next->Next;
      continue;
    }

    // "->" ident
    if(equal(Tok, "->")){
      // x -> y  ==  (*x).y
      Nd = newUnary(ND_DEREF, Nd, Tok);
      Nd = structRef(Nd, Tok->Next);
      Tok = Tok->Next->Next;
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

  Node Head = {};
  Node *Cur = &Head;

  while (!equal(Tok, ")"))
  {
    if (Cur != &Head)
      Tok = skip(Tok, ",");
    // assign
    Cur->Next = assign(&Tok, Tok);
    Cur = Cur->Next;
  }

  *Rest = skip(Tok, ")");

  Node *Nd = newNode(ND_FUNCALL, Start);
  // ident
  Nd->FuncName = strndup(Start->Loc, Start->Len);
  Nd->Args = Head.Next;
  return Nd;
}

// 解析括号、数字
// primary = "(" expr ")" | ident func-args? |  str | num
static Node *primary(Token **Rest, Token *Tok)
{
  // "(" "{" stmt + "}" ")"
  if (equal(Tok, "(") && equal(Tok->Next, "{")){
    
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
  
  // "sizeof" unary
  if (equal(Tok, "sizeof")) {
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
    // find variable from locals
    Obj *Var = findVar(Tok);
    // if no exist before, create one
    if (!Var)
    {
      errorTok(Tok, "undefined variable");
    }
    *Rest = Tok->Next;
    return newVarNode(Var, Tok);
  }

  // str
  if (Tok->Kind == TK_STR) {
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

// add params to the Locals
// reverse ?
static void createParamLVars(Type *Param) {
  if (Param){
    createParamLVars(Param->Next);
    // add to local
    newLVar(getIdent(Param->Name), Param);
  }
}

// functionDefinition = declspec declarator? ident "(" ")" "{" compoundStmt*
static Token *function(Token *Tok, Type *BaseTy) {
  Type *Ty = declarator(&Tok, Tok, BaseTy);

  Obj *Fn = newGVar(getIdent(Ty->Name), Ty);
  Fn -> isFunction = true;

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
  return Tok;
}

// build global variable
static Token *globalVariable(Token *Tok, Type *Basety)
{
  bool First = true;

  while (!consume(&Tok, Tok, ";"))
  {
    if(!First) {
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
  if(equal(Tok, ";"))
  {
    return false;
  }

  Type Dummy =  {};
  Type* Ty = declarator(&Tok, Tok, &Dummy);
  return Ty->Kind == TY_FUNC;
}


// syntax parser entry function
// program = (functionDefinition* | global-variable)*
Obj *parse(Token *Tok)
{
  Globals = NULL;

  while (Tok->Kind != TK_EOF)
  {
    Type* BaseTy = declspec(&Tok, Tok);
    
    // function
    if (isFunction(Tok)) {
      Tok = function(Tok, BaseTy);
      continue;
    }

    // global variable
    Tok = globalVariable(Tok, BaseTy);
  }

  return Globals;
}