#include "frontend/TypeChecker.h"
#include <cstdio>

/// unify - Applies the promotion rule between two types
Type TypeChecker::unify(Type A, Type B, char Op) {
    // Equal types: result is the same kind
    if (A == B)
        return A;

    // Int op Double or Double op Int: promotes to Double
    if ((A == Type::Int && B == Type::Double) ||
        (A == Type::Double && B == Type::Int))
        return Type::Double;

    // Any other pair is incompatible
    fprintf(stderr, "Type error: incompatible types in operation '%c'\n", Op);
    return Type::Unknown;
}

/// check - Solves the type of any node of the AST recursively
Type TypeChecker::check(ExprAST *E) {
    if (!E) {
        fprintf(stderr, "Type error: null node\n");
        return Type::Unknown;
    }

    switch (E->getKind()) {

    case NodeKind::NumberExpr:
        return Type::Double;

    case NodeKind::IntExpr:
        return Type::Int;

    case NodeKind::VariableExpr: {
        auto *V = static_cast<VariableExprAST *>(E);
        if (V->getType() == Type::Unknown)
            return Type::Double;
        return V->getType();
    }

    case NodeKind::BinaryExpr: {
        auto *B = static_cast<BinaryExprAST *>(E);
        Type LType = check(B->getLHS());
        Type RType = check(B->getRHS());
        Type Result = unify(LType, RType, B->getOp());
        B->setType(Result);
        return Result;
    }

    case NodeKind::CallExpr:
        return Type::Unknown;

    case NodeKind::IfExpr: {
        auto *I = static_cast<IfExprAST *>(E);

        // Verifies the type of condition - must be numeric
        Type CondType = check(I->getCond());
        if (CondType == Type::Unknown) {
            fprintf(stderr, "Type error: if condition with unknown type\n");
            return Type::Unknown;
        }

        // Verifies the type of both branches
        Type ThenType = check(I->getThen());
        Type ElseType = check(I->getElse());

        // Unifies the type of both branches
        Type Result = unify(ThenType, ElseType, '?');
        I->setType(Result);
        return Result;
    }

    default:
        fprintf(stderr, "Type error: unknown node\n");
        return Type::Unknown;
    }
}