#include "TypeChecker.h"
#include <stdexcept>
#include <string>

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
    std::string msg = "incompatible types in operation '";
    msg += Op;
    msg += "'";
    throw std::runtime_error(msg);
}

/// check - Solves the type of any node of the AST recursively
Type TypeChecker::check(ExprAST *E) {
    // Literal double: imediate type, doesn't need to inspetionate sons
    if (auto *N = dynamic_cast<NumberExprAST *>(E))
        return N->getType();

    // Literal int: immediate type
    if (auto *N = dynamic_cast<IntExprAST *>(E))
        return N->getType();

    // Variable: for now returns the type that it's already in the node
    // SOON: Symbol Table
    if (auto *V = dynamic_cast<VariableExprAST *>(E)) {
        if (V->getType() == Type::Unknown)
            throw std::runtime_error(
                "variable '" + std::string(V->getName()) +
                "' with unknown type - Symbol Table not implemented yet");
        return V->getType();
    }

    // Binary Expr: solves both sides recursively and unify types
    if (auto *B = dynamic_cast<BinaryExprAST *>(E)) {
        Type LType = check(B->getLHS());
        Type RType = check(B->getRHS());
        Type Result = unify(LType, RType, B->getOp());
        B->setType(Result); // fills the solved types in the node
        return Result;
    }

    // Function call: for now, return Unknown.
    // SOON: Will be solved with Symbol Table.
    if (auto *C = dynamic_cast<CallExprAST *>(E))
        return Type::Unknown;

    throw std::runtime_error("unknown node expression in type-checker");
}