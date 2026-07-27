#ifndef LEXER_H
#define LEXER_H

#include <string>

enum Token {
    tok_eof = -1,

    // commands
    tok_def = -2,
    tok_extern = -3,

    // primary
    tok_identifier = -4,
    tok_number = -5,
    tok_integer = -6
};

/// gettok  - Return the next token from standard input
extern std::string IdentifierStr; // Filled in if tok_identifier
extern double NumVal; // Filler in if tok_number
extern long long IntVal; // Filler in if tok_integer

int gettok();

#endif