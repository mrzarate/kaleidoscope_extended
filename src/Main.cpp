#include "Driver.h"
#include "frontend/Parser.h"
#include "codegen/CodeGen.h"
#include <cstdio>

int main() {
    // Define precedence of BinOps
    BinopPrecedence['<'] = 10;
    BinopPrecedence['+'] = 20;
    BinopPrecedence['-'] = 20;
    BinopPrecedence['*'] = 40;

    getNextToken();

    InitializeModule("kaleidoscope_extended");

    // Enters the main loop
    MainLoop();
    return 0;
}