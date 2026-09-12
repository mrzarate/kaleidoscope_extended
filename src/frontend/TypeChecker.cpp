#include "frontend/TypeChecker.h"
#include "llvm/Support/raw_ostream.h"

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
    llvm::errs() << "Type error: incompatible types in operation '"
                 << Op << "'\n";
    return Type::Unknown;
}

/// check - Solves the type of any node of the AST recursively
Type TypeChecker::check(ExprAST *E) {
    if (!E) {
        llvm::errs() << "Type error: null node\n";
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
            llvm::errs() << "Type error: if condition with unknown type\n";
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

    case NodeKind::VarDeclExpr: {
        auto *V = static_cast<VarDeclExprAST *>(E);
        Type InitType = check(V->getInit());
        // verifies if the initializator type is compatible
        // with the declared type
        if (InitType != V->getType() &&
            InitType != Type::Unknown) {
            // automatic promotion: int -> double if necessary
            if (!(InitType == Type::Int &&
                  V->getType() == Type::Double)) {
                llvm::errs() << "Type error: incompatible initializator type"
                             << " with declared type of '"
                             << V->getName() << "'\n";
            }
        }
        return V->getType();
    }

    case NodeKind::AssignExpr: {
        auto *A = static_cast<AssignExprAST *>(E);
        Type ValType = check(A->getValue());
        // the assignment type is the type of the value
        // the check against the target variable will be performed
        // once the symbol table is integrated
        A->setType(ValType);
        return ValType;
    }

    case NodeKind::BlockExpr: {
        auto *Block = static_cast<BlockExprAST *>(E);
        Type LastType = Type::Unknown;
        for (auto &Stmt : Block->getStmts())
            LastType = check(Stmt.get());
        // type of the block is the type of the last statement
        Block->setType(LastType);
        return LastType;
    }

    default:
        llvm::errs() << "Type error: unknown node\n";
        return Type::Unknown;
    }
}