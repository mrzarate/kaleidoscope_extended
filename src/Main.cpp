#include "Driver.h"
#include "frontend/Parser.h"
#include "codegen/CodeGen.h"
#include <cstdio>
#include <unistd.h>

int main(int argc, char *argv[]) {
    // Verifies if a file was passed as argument
    if (argc < 2) {
        fprintf(stderr, "Use: %s <file.ks> [-o <output.o>]\n", argv[0]);
        fprintf(stderr, "Ex: %s program.ks -o program.o\n", argv[0]);
        return 1;
    }

    // Opens the file src.ks
    FILE *InputFile = fopen(argv[1], "r");
    if (!InputFile) {
        fprintf(stderr, "Error: was not possible open the file '%s'\n", argv[1]);
        return 1;
    }
    dup2(fileno(InputFile), STDIN_FILENO);
    fclose(InputFile);

    // Determins the name of the output file
    // Standard: same name as source file, with extension .o
    std::string OutputFilename = "output.o";
    for (int i = 2; i < argc; i++) {
        if (std::string(argv[i]) == "-o" && i + 1 < argc) {
            OutputFilename = argv[i + 1];
            break;
        }
    }

    // Define precedence of BinOps
    BinopPrecedence['<'] = 10;
    BinopPrecedence['+'] = 20;
    BinopPrecedence['-'] = 20;
    BinopPrecedence['*'] = 40;

    // Initializes LLVM Module and otimization managers
    InitializeModuleAndManagers();

    // Reads the first token and enters the main loop
    getNextToken();
    MainLoop();

    // Emits object file
    if (!EmitObjectFile(OutputFilename))
        return 1;
    
    return 0;
}