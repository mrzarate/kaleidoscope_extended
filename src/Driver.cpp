#include "Driver.h"
#include "Lexer.h"
#include "Ast.h"
#include <cstdio>

/// Top-Level Parsing

static void HandleDefinition() {
    if (!ParseDefinition()) {
        getNextToken(); // skip token for error recovery
    }
}

static void HandleExtern() {
    if (!ParseExtern()) {
        getNextToken(); // skip token for error recovery
    }
}

static void HandleTopLevelExpression() {
    if (!ParseTopLevelExpr()) {
        getNextToken(); // skip token for error recovery
    }
}

/// top ::= definition | external | expression | ';'
void MainLoop() {
    while(true) {
        fprintf(stderr, "ready> ");
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