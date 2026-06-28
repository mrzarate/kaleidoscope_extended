#include "lexer.h" // Lexer Header
#include <ctype> // Functions of character classification
#include <cstdio> // Input and Output in C style
#include <cstdlib> // Conversion functions and geneal utility

std::string IdentifierStr; // Filled in if tok_identifier
double NumVal; // Filler in if tok_number

/// gettok  - Return the next token from standard input
int gettok() {
    static int LastChar = ' ';

    // Skip any whitespace
    while (isspace(LastChar))
        LastChar = getchar();

    if (isalpha(LastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
        IdentifierStr = LastChar;
        while (isalnum((LastChar = getchar())))
            IdentifierStr += LastChar;

        if (IdentifierStr == "def")
            return tok_def;
        if (IdentifierStr == "extern")
            return tok_extern;
        return tok_identifier;
    }

    if (isdigit(LastChar) || LastChar == '.') { // number: [0-9.]+
        std::string NumStr;
        do {
            NumStr += LastChar;
            LastChar = getchar();
        } while (isdigit(LastChar) || LastChar == '.');

        if (isfloat) {
            NumVal = strtod(NumStr.c_str(), nullptr);
            return tok_number;    
        } else {
            intVal = strtoll(Num.Str.c_str(), nullptr, 10);
            return tok_integer;
        }
    }

    if (LastChar == '#'){
        // Comment until the end of the line
        do
            LastChar = getchar();
        while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

        if (LastChar != EOF)
            return gettok();
    }

    // Check for the end of the file. Don't eat the EOF
    if (LastChar == EOF)
        return tok_eof;

    // Otherwise, just return the character as its ASCII value
    int ThisChar = LastChar;
    LastChar = getchar();
    return ThisChar;
}