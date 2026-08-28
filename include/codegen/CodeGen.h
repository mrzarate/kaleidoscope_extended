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
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/IR/PassManager.h"

// Required headers to emit object file
#include "llvm/Support/FileSystem.h" // Opening of output file
#include "llvm/Support/TargetSelect.h" // Inicialization of native target
#include "llvm/Target/TargetMachine.h" // Target machine
#include "llvm/Target/TargetOptions.h" // Target Options
#include "llvm/TargetParser/Host.h" // Host's CPU detection
#include "llvm/MC/TargetRegistry.h" // register of available targets

#include <map>
#include <memory>
#include <string>

// Main goals of codegen
extern std::unique_ptr<llvm::LLVMContext> TheContext;
extern std::unique_ptr<llvm::Module> TheModule;
extern std::unique_ptr<llvm::IRBuilder<>> Builder;
extern std::map<std::string, llvm::Value *> NamedValues;

// Otimization managers
extern std::unique_ptr<llvm::FunctionPassManager> TheFPM;
extern std::unique_ptr<llvm::LoopAnalysisManager> TheLAM;
extern std::unique_ptr<llvm::FunctionAnalysisManager> TheFAM;
extern std::unique_ptr<llvm::CGSCCAnalysisManager> TheCGAM;
extern std::unique_ptr<llvm::ModuleAnalysisManager> TheMAM;

llvm::Value *LogErrorV(const char *Str);
void InitializeModuleAndManagers();

bool EmitObjectFile(const std::string &Filename);

#endif // CODEGEN_H