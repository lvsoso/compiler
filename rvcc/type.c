#include "rvcc.h"

Type *TyVoid = &(Type){TY_VOID, 1, 1};
Type *TyBool = &(Type){TY_BOOL, 1, 1};

// {TY_INT}构造了一个数据结构，(Type)强制类型转换为struct，然后&取地址
// 全局变量TyInt，用来将Type赋值为int类型
Type *TyInt = &(Type){TY_INT, 4, 4};
Type *TyLong = &(Type){TY_LONG, 8, 8};
Type *TyShort = &(Type){TY_SHORT, 2, 2};

Type *TyChar = &(Type){TY_CHAR, 1, 1};


static Type *newType(TypeKind Kind, int Size, int Align){
    Type *Ty = calloc(1, sizeof(Type));
    Ty->Kind = Kind;
    Ty->Size = Size;
    Ty->Align = Align;
    return Ty;
}

// judge the 'type' is integer
bool isInteger(Type *Ty) {
  TypeKind K = Ty->Kind;
  return K == TY_BOOL || K == TY_CHAR || K == TY_SHORT || K == TY_INT ||
         K == TY_LONG || K == TY_ENUM;
}

// copy type
Type *copyType(Type *Ty) {
  Type *Ret = calloc(1, sizeof(Type));
  *Ret = *Ty;
  return Ret;
}

Type *pointerTo(Type *Base)
{
    Type *Ty = newType(TY_PTR, 8, 8);
    Ty->Base = Base;
    return Ty;
}

// function type
Type *funcType(Type *ReturnTy) {
  Type *Ty = calloc(1, sizeof(Type));
  Ty->Kind = TY_FUNC;
  Ty->ReturnTy = ReturnTy;
  return Ty;
}

// build 'array' type, pass the base of array, num of elements
Type *arrayOf(Type *Base, int Len)
{
    Type *Ty = newType(TY_ARRAY, Base->Size * Len, Base->Align);
    Ty->Base = Base;
    Ty->ArrayLen = Len;
    return Ty;
}

Type *enumType(void) { return newType(TY_ENUM, 4, 4); }

Type *structType(void) { return newType(TY_STRUCT, 0, 1); }

static Type *getCommonType(Type *Ty1, Type * Ty2){
    if(Ty1->Base){
        return pointerTo(Ty1->Base);
    }
  if (Ty1->Size == 8 || Ty2->Size == 8){
        return TyLong;
    }
  return TyInt;
}

static void usualArithConv(Node **LHS, Node **RHS) {
  Type *Ty = getCommonType((*LHS)->Ty, (*RHS)->Ty);
  *LHS = newCast(*LHS, Ty);
  *RHS = newCast(*RHS, Ty);
}


void addType(Node *Nd)
{
    if (!Nd || Nd->Ty)
        return;

    // recursive access all node for add type
    addType(Nd->LHS);
    addType(Nd->RHS);
    addType(Nd->Cond);
    addType(Nd->Then);
    addType(Nd->Els);
    addType(Nd->Init);
    addType(Nd->Inc);

    for (Node *N = Nd->Body; N; N = N->Next)
    {
        addType(N);
    }

    // recursive access all args
    for (Node *N = Nd->Args; N; N = N->Next)
    {
    addType(N);
    }
    
    switch (Nd->Kind)
    {
    case ND_NUM:
        Nd->Ty = (Nd->Val == (int)Nd->Val) ? TyInt : TyLong;
        return;
    // 节将节点类型设为 节点左部的类型
    case ND_ADD:
    case ND_SUB:
    case ND_MUL:
    case ND_DIV:
    case ND_MOD:
    case ND_BITAND:
    case ND_BITOR:
    case ND_BITXOR:
        usualArithConv(&Nd->LHS, &Nd->RHS);
        Nd->Ty = Nd->LHS->Ty;
        return;
    case ND_NEG: {
            Type *Ty = getCommonType(TyInt, Nd->LHS->Ty);
            Nd->LHS = newCast(Nd->LHS, Ty);
            Nd->Ty = Ty;
            return;
        }
    case ND_ASSIGN:
        if (Nd->LHS->Ty->Kind == TY_ARRAY){
            errorTok(Nd->LHS->Tok, "not an lvalue");
        }
        if (Nd->LHS->Ty->Kind != TY_STRUCT){
            Nd->RHS = newCast(Nd->RHS, Nd->LHS->Ty);
        }
        Nd->Ty = Nd->LHS->Ty;
        return;
    // 将节点类型设为 int
    case ND_EQ:
    case ND_NE:
    case ND_LT:
    case ND_LE:
        usualArithConv(&Nd->LHS, &Nd->RHS);
        Nd->Ty = TyInt;
        return;
    case ND_FUNCALL:
        Nd->Ty = TyLong;
        return;
    case ND_NOT:
    case ND_LOGOR:
    case ND_LOGAND:
        Nd->Ty = TyInt;
        return;
    case ND_BITNOT:
    case ND_SHL:
    case ND_SHR:
        Nd->Ty = Nd->LHS->Ty;
        return;
    // set variable's type as  Node'type
    case ND_VAR:
        Nd->Ty = Nd->Var->Ty;
        return;

    // 将节点类型设为 右部的类型?
    case ND_COMMA:
        Nd->Ty = Nd->RHS->Ty;
        return;
    //  将节点类型设置为成员的类型
    case ND_MEMBER:
        Nd->Ty = Nd -> Mem->Ty;
        return;
    // 将节点类型设为 指针，并指向左部的类型
    case ND_ADDR:
        if (Nd->LHS->Ty->Kind == TY_ARRAY)
        {
            Nd->Ty = pointerTo(Nd->LHS->Ty->Base);
        }
        else
        {
            Nd->Ty = pointerTo(Nd->LHS->Ty);
        }
        return;

    // 节点类型：如果解引用指向的是指针，则为指针指向的类型；否则为int
    case ND_DEREF:
        if (!Nd->LHS->Ty->Base)
        {
            errorTok(Nd->Tok, "invalid pointer dereference");
        }
        // 空指针不能解引用
        if (Nd->LHS->Ty->Base->Kind == TY_VOID)
        {
            errorTok(Nd->Tok, "dereferencing a void pointer");
        }
        Nd->Ty = Nd->LHS->Ty->Base;
        return;
     case ND_STMT_EXPR:
     if (Nd -> Body) {
        Node *Stmt = Nd -> Body;
        while(Stmt->Next)
        {
            Stmt = Stmt->Next;
        }
        if (Stmt->Kind == ND_EXPR_STMT){
                    Nd->Ty = Stmt->LHS->Ty;
        return;
        }
        }
        errorTok(Nd->Tok, "statement expression returning void is not supported");
    return;
    default:
        break;
    }
}