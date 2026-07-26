#include "Driver.h"
#include "Lexer.h"
#include "Parser.h"
#include "Ast.h"
#include "TypeChecker.h"
#include <cstdio>

static TypeChecker TC;

/// Top-Level Parsing

static void HandleDefinition() {
    if (auto FunctionAST = ParseDefinition()) {
        try {
            TC.check(FunctionAST->getBody());
        } catch (const std::runtime_error &e) {
            fprintf(stderr, "Type error: %s\n", e.what());
        }
    } else {
        getNextToken(); // skip token for error recovery
    }
}

static void HandleExtern() {
    if (!ParseExtern()) {
        getNextToken(); // skip token for error recovery
    }
}

static void HandleTopLevelExpression() {
    if (auto ExprAST = ParseTopLevelExpr()) {
        try {
            TC.check(ExprAST->getBody());
        } catch (const std::runtime_error &e) {
            fprintf(stderr, "Type error: %s\n", e.what());
        }
    } else {
        getNextToken(); // skip token for error recovery
    }
}

/// top ::= definition | external | expression | ';'
void MainLoop() {
    while(true) {
        switch (CurTok) {
            case tok_eof:
                return;
            case ';': // ignore top-level semicolons
                getNextToken();
                break;
            case tok_def:
                HandleDefinition();
                break;
            case tok_extern:
                HandleExtern();
                break;
            default:
                HandleTopLevelExpression();
                break;
        }
    }
}