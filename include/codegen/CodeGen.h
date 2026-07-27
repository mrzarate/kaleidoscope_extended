#ifndef CODEGEN_H
#define CODEGEN_H

#include "Ast.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Constants.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include <map>
#include <memory>
#include <string>

extern std::unique_ptr<llvm::LLVMContext> TheContext;
extern std::unique_ptr<llvm::Module> TheModule;
extern std::unique_ptr<llvm::IRBuilder<>> Builder;
extern std::map<std::string, llvm::Value *> NamedValues;

llvm::Value *LogErrorV(const char *Str);
void InitializeModule(const std::string &ModuleName);

#endif // CODEGEN_H