#ifndef PARSER_H
#define PARSER_H

#include "Ast.h"
#include "frontend/Lexer.h"
#include <map>
#include <memory>

extern int CurTok;

extern std::map<char, int> BinopPrecedence;

int getNextToken();

std::unique_ptr<ExprAST> LogError(const char *Str);
std::unique_ptr<PrototypeAST> LogErrorP(const char *Str);

std::unique_ptr<FunctionAST> ParseDefinition();
std::unique_ptr<PrototypeAST> ParseExtern();
std::unique_ptr<FunctionAST> ParseTopLevelExpr();

#endif // PARSER_H