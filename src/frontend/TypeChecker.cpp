#include "frontend/TypeChecker.h"
#include <cstdio>

/// unify - Applies the promotion rule between two types
ASTType TypeChecker::unify(ASTType A, ASTType B, char Op) {
    // Equal types: result is the same kind
    if (A == B)
        return A;

    // Int op Double or Double op Int: promotes to Double
    if ((A == ASTType::Int && B == ASTType::Double) ||
        (A == ASTType::Double && B == ASTType::Int))
        return ASTType::Double;

    // Any other pair is incompatible
    llvm::errs() << "Type error: incompatible types in operation '"
                 << Op << "'\n";
    return ASTType::Unknown;
}

/// check - Solves the type of any node of the AST recursively
ASTType TypeChecker::check(ExprAST *E) {
    if (!E) {
        llvm::errs() << "Type error: null node\n";
        return ASTType::Unknown;
    }

    switch (E->getKind()) {

    case NodeKind::NumberExpr:
        return ASTType::Double;

    case NodeKind::IntExpr:
        return ASTType::Int;

    case NodeKind::VariableExpr: {
        auto *V = static_cast<VariableExprAST *>(E);
        if (V->getType() == ASTType::Unknown)
            return ASTType::Double;
        return V->getType();
    }

    case NodeKind::BinaryExpr: {
        auto *B = static_cast<BinaryExprAST *>(E);
        ASTType LType = check(B->getLHS());
        ASTType RType = check(B->getRHS());
        ASTType Result = unify(LType, RType, B->getOp());
        B->setType(Result);
        return Result;
    }

    case NodeKind::CallExpr:
        return ASTType::Unknown;

    case NodeKind::IfExpr: {
        auto *I = static_cast<IfExprAST *>(E);

        // Verifies the type of condition - must be numeric
        ASTType CondType = check(I->getCond());
        if (CondType == ASTType::Unknown) {
            fprintf(stderr, "ASTType error: if condition with unknown type\n");
            return ASTType::Unknown;
        }

        // Verifies the type of both branches
        ASTType ThenType = check(I->getThen());
        ASTType ElseType = check(I->getElse());

        // Unifies the type of both branches
        ASTType Result = unify(ThenType, ElseType, '?');
        I->setType(Result);
        return Result;
    }

    case NodeKind::VarDeclExpr: {
        auto *V = static_cast<VarDeclExprAST *>(E);
        ASTType InitType = check(V->getInit());
        if (InitType != V->getType() && InitType != ASTType::Unknown)
            if (!(InitType == ASTType::Int && V->getType() == ASTType::Double))
                llvm::errs() << "Type error: incompatible type in declaration of '"
                             << V->getName() << "'\n";
        return V->getType();
    }

     case NodeKind::AssignExpr: {
        auto *A = static_cast<AssignExprAST *>(E);
        ASTType ValType = check(A->getValue());
        A->setType(ValType);

        return ValType;
    }

    case NodeKind::BlockExpr: {
        auto *Block = static_cast<BlockExprAST *>(E);
        ASTType LastType = ASTType::Unknown;
        for (auto &Stmt: Block->getStmts())
            LastType = check(Stmt.get());
        Block->setType(LastType);
        
        return LastType;
    }

    default:
        llvm::errs() << "ASTType error: unknown node\n";
        return ASTType::Unknown;
    }
}