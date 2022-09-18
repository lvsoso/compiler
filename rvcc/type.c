#include "rvcc.h"

// {TY_INT}构造了一个数据结构，(Type)强制类型转换为struct，然后&取地址
// 全局变量TyInt，用来将Type赋值为int类型
Type *TyInt = &(Type){TY_INT};

bool isInteger(Type *Ty) { return Ty->Kind == TY_INT; }

Type *pointerTo(Type *Base)
{
    Type *Ty = calloc(1, sizeof(Type));
    Ty->Kind = TY_PTR;
    Ty->Base = Base;
    return Ty;
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

    switch (Nd->Kind)
    {
    // 节将节点类型设为 节点左部的类型
    case ND_ADD:
    case ND_SUB:
    case ND_MUL:
    case ND_DIV:
    case ND_NEG:
    case ND_ASSIGN:
        Nd->Ty = Nd->LHS->Ty;
        return;
        
    // 将节点类型设为 int
    case ND_EQ:
    case ND_NE:
    case ND_LT:
    case ND_LE:
    case ND_NUM:
    case ND_FUNCALL:
        Nd->Ty = TyInt;
        return;
    // set variable's type as  Node'type
    case ND_VAR:
        Nd->Ty = Nd->Var->Ty;
        return;
    // 将节点类型设为 指针，并指向左部的类型
    case ND_ADDR:
        Nd->Ty = pointerTo(Nd->LHS->Ty);
        return;

    // 节点类型：如果解引用指向的是指针，则为指针指向的类型；否则为int
    case ND_DEREF:
        if (Nd->LHS->Ty->Kind != TY_PTR)
        {
            errorTok(Nd->Tok, "invalid pointer dereference");
        }
        Nd->Ty = Nd->LHS->Ty->Base;
        return;
    default:
        break;
    }
}