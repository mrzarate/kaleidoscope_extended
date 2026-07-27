#include "codegen/CodeGen.h"
#include "Ast.h"
#include <cstdio>

using namespace llvm;

/// Global variables of code generation

std::unique_ptr<LLVMContext> TheContext;
std::unique_ptr<Module> TheModule;
std::unique_ptr<IRBuilder<>> Builder;
std::map<std::string, Value *> NamedValues;

Value *LogErrorV(const char *Str) {
    fprintf(stderr, "Error: %s\n", Str);
    return nullptr;
}

void InitializeModule(const std::string &ModuleName) {
    TheContext = std::make_unique<LLVMContext>();
    TheModule = std::make_unique<Module>(ModuleName, *TheContext);
    Builder = std::make_unique<IRBuilder<>>(*TheContext);
}

/// Codegen Expressions
/// Literal Double: creates constant of FP
Value *NumberExprAST::codegen() {
    return ConstantFP::get(*TheContext, APFloat(Val));
}

/// Literal Integer: creates constant int of 64 bits signed
Value *IntExprAST::codegen() {
    return ConstantInt::get(*TheContext, APInt(64, getVal(), true));
}

/// Reference to variable: search in the value map of current scope
Value *VariableExprAST::codegen() {
    Value *V = NamedValues[Name];
    if (!V)
        return LogErrorV("Unknown variable name");
    return V;
}

/// Binary Op: creates correct instruction based on solved type
Value *BinaryExprAST::codegen() {
    Value *L = LHS->codegen();
    Value *R = RHS->codegen();
    if (!L || !R)
        return nullptr;

    // if one side int int and the ohter double, promotes int to double
    if (L->getType()->isIntegerTy() && R->getType()->isDoubleTy())
        L = Builder->CreateSIToFP(L, llvm::Type::getDoubleTy(*TheContext), "conv");
    else if (L->getType()->isDoubleTy() && R->getType()->isIntegerTy())
        R = Builder->CreateSIToFP(R, llvm::Type::getDoubleTy(*TheContext), "conv");

    bool isInt = L->getType()->isIntegerTy();

    switch(Op) {
    case '+':
        return isInt ? Builder->CreateAdd(L, R, "addtmp")
                     : Builder->CreateFAdd(L, R, "addtmp");
    case '-':
        return isInt ? Builder->CreateSub(L, R, "subtmp")
                     : Builder->CreateFSub(L, R, "subtmp");
    case '*':
        return isInt ? Builder->CreateMul(L, R, "multmp")
                     : Builder->CreateFMul(L, R, "multmp");
    case '<':
        if (isInt) {
            L = Builder->CreateICmpSLT(L, R, "cmptmp");
            // converts bool (i1) to i64
            return Builder->CreateZExt(L, llvm::Type::getInt64Ty(*TheContext), "booltmp");
        } else {
            L = Builder->CreateFCmpULT(R, L, "cmptmp");
            // converts bool (i1) to double
            return Builder->CreateUIToFP(L, llvm::Type::getDoubleTy(*TheContext), "booltmp");
        }
    default:
        return LogErrorV("invalid binary operator");
    }
}

/// Function call
Value *CallExprAST::codegen() {
    Function *CalleeF = TheModule->getFunction(Callee);
    if (!CalleeF)
        return LogErrorV("Unknown function referenced");

    if (CalleeF->arg_size() != Args.size())
        return LogErrorV("Incorrect # arguments passed");

    std::vector<Value *> ArgsV;
    for (unsigned i = 0, e = Args.size(); i != e; ++i) {
        ArgsV.push_back(Args[i]->codegen());
        if (!ArgsV.back())
            return nullptr;
    }

    return Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

/// Prototype: declares a functionin in the LLVM module
Function *PrototypeAST::codegen() {
    // SOON: Uses the correct type for each parameter
    std::vector<llvm::Type *> Doubles(Args.size(),
                                llvm::Type::getDoubleTy(*TheContext));

    FunctionType *FT = FunctionType::get(
        llvm::Type::getDoubleTy(*TheContext), Doubles, false);

    Function *F = Function::Create(FT, Function::ExternalLinkage,
                                    Name, TheModule.get());

    unsigned Idx = 0;
    for (auto &Arg : F->args())
        Arg.setName(Args[Idx++]);

    return F;
}

/// Complete funtion definition
Function *FunctionAST::codegen() {
    Function *TheFunction = TheModule->getFunction(Proto->getName());

    if (!TheFunction)
        TheFunction = Proto->codegen();

    if (!TheFunction)
        return nullptr;

    if (!TheFunction->empty())
        return (Function *)LogErrorV("Function cannot be redefined.");

    // Creates the entry block of the function
    BasicBlock *BB = BasicBlock::Create(*TheContext, "entry", TheFunction);
    Builder->SetInsertPoint(BB);

    // Register the parameters in the value map
    NamedValues.clear();
    for (auto &Arg :TheFunction->args())
        NamedValues[std::string(Arg.getName())] = &Arg;

    if (Value *RetVal = Body->codegen()) {
        Builder->CreateRet(RetVal);
        verifyFunction(*TheFunction);
        return TheFunction;
    }

    TheFunction->eraseFromParent();
    return nullptr;
}