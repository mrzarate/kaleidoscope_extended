#include "Driver.h"
#include "Parser.h"
#include <cstdio>

int main() {
    // Define precedence of BinOps
    BinopPrecedence['<'] = 10;
    BinopPrecedence['+'] = 20;
    BinopPrecedence['-'] = 20;
    BinopPrecedence['*'] = 40;

    getNextToken();

    // Enters the main loop
    MainLoop();
    return 0;
}