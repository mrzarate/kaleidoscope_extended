#include "Ast.h"
#include "Driver.h"
#include "frontend/Lexer.h"
#include "frontend/Parser.h"
#include "frontend/TypeChecker.h"
#include "codegen/CodeGen.h"
#include <cstdio>

using namespace llvm;

static TypeChecker TC;

/// Top-Level Parsing

static void HandleDefinition() {
    if (auto FunctionAST = ParseDefinition()) {
        TC.check(FunctionAST->getBody());
        if (!FunctionAST->codegen()) {
            fprintf(stderr, "Error: fail to generate code for function definition.\n");
        }
    } else {
        getNextToken(); // skip token for error recovery
    }
}

static void HandleExtern() {
    if (auto ProtoAST = ParseExtern()) {
        if (!ProtoAST->codegen()) {
            fprintf(stderr, "Error: fail to generate code for extern.\n");
        }
    } else {
        getNextToken(); // skip token for error recovery
    }
}

static void HandleTopLevelExpression() {
    if (auto FunctionAST = ParseTopLevelExpr()) {
        TC.check(FunctionAST->getBody());
        if (!FunctionAST->codegen()) {
            fprintf(stderr, "Error: fail to generate code for expression.\n");
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