#ifndef TYPECHECKER_H
#define TYPECHECKER_H

#include "Ast.h"

/// Runs through the AST e solves the type of each node
/// Until now, it only supports Int or Double
class TypeChecker {
public:
    /// Receive any node of expression, solves it recursively
    /// and returns the resultant type.
    Type check(ExprAST *E);

private:
    /// Apply the promotion rule between two types.
    Type unify(Type A, Type B, char Op);
};

#endif // TYPECHECKER_H