#include "rvcc.h"

// save local variables
Obj *Locals;


// program = "{" compoundStmt
// compoundStmt = stmt* "}"
// stmt = "return" expr ";"
//        | "if" "(" expr ")" stmt ("else" stmt )?
//        | "{" compoundStmt
//        | exprStmt
// exprStmt = expr? ";"
// expr = assign
// assign = equality ("=" assign)?
// equality = relational ("==" relational | "!=" relational)*
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
// add = mul ("+" mul | "-" mul)*
// mul = unary ("*" unary | "/" unary)*
// unary = ("+" | "-") unary | primary
// primary = "(" expr ")" | ident | num
static Node *compoundStmt(Token **Rest, Token *tok);
static Node *exprStmt(Token **Rest, Token *Tok);
static Node *expr(Token **Rest, Token *Tok);
static Node *assign(Token **Rest, Token *Tok);
static Node *equality(Token **Rest, Token *Tok);
static Node *relational(Token **Rest, Token *Tok);
static Node *add(Token **Rest, Token *Tok);
static Node *mul(Token **Rest, Token *Tok);
static Node *unary(Token **Rest, Token *Tok);
static Node *primary(Token **Rest, Token *Tok);

// find local variable by name
static Obj *findVar(Token *Tok) {
  // for-loop locals variable
  for (Obj *Var = Locals; Var; Var = Var->Next)
    // compare name
    if (strlen(Var->Name) == Tok->Len &&
        !strncmp(Tok->Loc, Var->Name, Tok->Len))
      return Var;
  return NULL;
}

// new a node
static Node *newNode(NodeKind Kind)
{
  Node *Nd = calloc(1, sizeof(Node));
  Nd->Kind = Kind;
  return Nd;
}

// new single tree node
static Node *newUnary(NodeKind Kind, Node *Expr)
{
  Node *Nd = newNode(Kind);
  Nd->LHS = Expr;
  return Nd;
}

// new a binary node
static Node *newBinary(NodeKind Kind, Node *LHS, Node *RHS)
{
  Node *Nd = newNode(Kind);
  Nd->LHS = LHS;
  Nd->RHS = RHS;
  return Nd;
}

//  new a number node
static Node *newNum(int Val)
{
  Node *Nd = newNode(ND_NUM);
  Nd->Val = Val;
  return Nd;
}

// new variable
static Node *newVarNode(Obj *Var)
{
  Node *Nd = newNode(ND_VAR);
  Nd->Var = Var;
  return Nd;
}

// insert new variable to 'locals'
static Obj *newLVar(char *Name)
{
  Obj *Var = calloc(1, sizeof(Obj));
  Var->Name = Name;
  // the new one be the head
  Var->Next = Locals;
  Locals = Var;
  return Var;
}

// stmt = "return" expr ";"
//        | "if" "(" expr ")" stmt ("else" stmt )?
//        | "{" compoundStmt
//        | exprStmt
static Node *stmt(Token **Rest, Token *Tok)
{
  // "return" expr ";"
  if (equal(Tok, "return")) {
    Node *Nd = newUnary(ND_RETURN, expr(&Tok, Tok->Next));
    *Rest = skip(Tok, ";");
    return Nd;
  }

  // "if" "(" expr ")" stmt ("else" stmt)?
  if (equal(Tok, "if")) {
    Node *Nd = newNode(ND_IF);
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

  // "{" compoundStmt
  if (equal(Tok, "{"))
    return compoundStmt(Rest, Tok->Next);

  // exprStmt
  return exprStmt(Rest, Tok);
}

// compoundStmt = stmt* "}"
static Node *compoundStmt(Token **Rest, Token *Tok){

  Node Head = {};
  Node *Cur = &Head;

  // stmt* "}"
  while (!equal(Tok, "}")){
    Cur -> Next = stmt(&Tok, Tok);
    Cur = Cur->Next;
  }


  // save {} block in Node's Body;
  Node *Nd = newNode(ND_BLOCK);
  Nd->Body = Head.Next;
  *Rest = Tok->Next;
  return Nd;
}


// exprStmt = expr? ";"
static Node *exprStmt(Token **Rest, Token *Tok)
{
  // ";"
  if (equal(Tok, ";")){
    *Rest = Tok->Next;
    return newNode(ND_BLOCK);
  }

  // expr ";"
  Node *Nd = newUnary(ND_EXPR_STMT, expr(&Tok, Tok));
  *Rest = skip(Tok, ";");
  return Nd;
}

// expr = assign
static Node *expr(Token **Rest, Token *Tok)
{
  return assign(Rest, Tok);
}

// assign = equality ("=" assign)?
static Node *assign(Token **Rest, Token *Tok)
{
  // equality
  Node *Nd = equality(&Tok, Tok);

  // 可能存在递归赋值，如a=b=1
  // ("=" assign)?
  if (equal(Tok, "="))
    Nd = newBinary(ND_ASSIGN, Nd, assign(&Tok, Tok->Next));
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
    // "==" relational
    if (equal(Tok, "=="))
    {
      Nd = newBinary(ND_EQ, Nd, relational(&Tok, Tok->Next));
      continue;
    }

    // "!=" relational
    if (equal(Tok, "!="))
    {
      Nd = newBinary(ND_NE, Nd, relational(&Tok, Tok->Next));
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
    // "<" add
    if (equal(Tok, "<"))
    {
      Nd = newBinary(ND_LT, Nd, add(&Tok, Tok->Next));
      continue;
    }

    // "<=" add
    if (equal(Tok, "<="))
    {
      Nd = newBinary(ND_LE, Nd, add(&Tok, Tok->Next));
      continue;
    }

    // ">" add
    // X>Y  ~ Y<X
    if (equal(Tok, ">"))
    {
      Nd = newBinary(ND_LT, add(&Tok, Tok->Next), Nd);
      continue;
    }

    // ">=" add
    // X>=Y ~ Y<=X
    if (equal(Tok, ">="))
    {
      Nd = newBinary(ND_LE, add(&Tok, Tok->Next), Nd);
      continue;
    }

    *Rest = Tok;
    return Nd;
  }
}
// add = mul ("+" mul | "-" mul)*
static Node *add(Token **Rest, Token *Tok)
{
  // mul
  Node *Nd = mul(&Tok, Tok);

  // ("+" mul | "-" mul)*
  while (true)
  {
    // "+" mul
    if (equal(Tok, "+"))
    {
      Nd = newBinary(ND_ADD, Nd, mul(&Tok, Tok->Next));
      continue;
    }

    // "-" mul
    if (equal(Tok, "-"))
    {
      Nd = newBinary(ND_SUB, Nd, mul(&Tok, Tok->Next));
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
  // unary
  Node *Nd = unary(&Tok, Tok);

  // ("*" unary | "/" unary)*
  while (true)
  {
    // "*" unary
    if (equal(Tok, "*"))
    {
      Nd = newBinary(ND_MUL, Nd, unary(&Tok, Tok->Next));
      continue;
    }

    // "/" unary
    if (equal(Tok, "/"))
    {
      Nd = newBinary(ND_DIV, Nd, unary(&Tok, Tok->Next));
      continue;
    }

    *Rest = Tok;
    return Nd;
  }
}

// unary = ("+" | "-") unary | primary
static Node *unary(Token **Rest, Token *Tok)
{
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
// primary = "(" expr ")" | ident｜num
static Node *primary(Token **Rest, Token *Tok)
{
  // "(" expr ")"
  if (equal(Tok, "("))
  {
    Node *Nd = expr(&Tok, Tok->Next);
    *Rest = skip(Tok, ")");
    return Nd;
  }

  // ident
  if (Tok->Kind == TK_IDENT) 
  {
    // find variable from locals
    Obj *Var = findVar(Tok);
    // if no exist before, create one
    if (!Var)
      // strndup copy n 's  characters
      Var = newLVar(strndup(Tok->Loc, Tok->Len));
    *Rest = Tok->Next;
    return newVarNode(Var);
  }
  
  // num
  if (Tok->Kind == TK_NUM)
  {
    Node *Nd = newNum(Tok->Val);
    *Rest = Tok->Next;
    return Nd;
  }

  errorTok(Tok, "expected an expression");
  return NULL;
}

// syntax parser entry function
// program = "{" compoundStmt
Function *parse(Token *Tok)
{
   // "{"
  Tok = skip(Tok, "{");

  Function *Prog= calloc(1, sizeof(Function));
  Prog->Body = compoundStmt(&Tok, Tok);
  Prog->Locals = Locals; // save local variable
  return Prog;
}