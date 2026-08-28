#include "codegen/CodeGen.h"
#include "Ast.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/IR/LegacyPassManager.h"
#include <cstdio>

using namespace llvm;

/// Global variables of code generation

std::unique_ptr<LLVMContext> TheContext;
std::unique_ptr<Module> TheModule;
std::unique_ptr<IRBuilder<>> Builder;
std::map<std::string, Value *> NamedValues;

std::unique_ptr<FunctionPassManager> TheFPM;
std::unique_ptr<LoopAnalysisManager> TheLAM;
std::unique_ptr<FunctionAnalysisManager> TheFAM;
std::unique_ptr<CGSCCAnalysisManager> TheCGAM;
std::unique_ptr<ModuleAnalysisManager> TheMAM;

Value *LogErrorV(const char *Str) {
    fprintf(stderr, "Error: %s\n", Str);
    return nullptr;
}

void InitializeModuleAndManagers() {
    // Creates context, module and builder
    TheContext = std::make_unique<LLVMContext>();
    TheModule = std::make_unique<Module>("kaleidoscope_extended", *TheContext);
    Builder = std::make_unique<IRBuilder<>>(*TheContext);

    // Creates analysis and transformation managers
    TheFPM = std::make_unique<FunctionPassManager>();
    TheLAM = std::make_unique<LoopAnalysisManager>();
    TheFAM = std::make_unique<FunctionAnalysisManager>();
    TheCGAM = std::make_unique<CGSCCAnalysisManager>();
    TheMAM = std::make_unique<ModuleAnalysisManager>();

    // Adds otimization passes per function
    // Peephole : local simplification of instructions
    TheFPM->addPass(InstCombinePass());
    // Reassociation: reorders expressions to expose them to more otimizations
    TheFPM->addPass(ReassociatePass());
    // GVN: eliminate common subexpressions
    TheFPM->addPass(GVNPass());
    // SimplifyCFG: remove dead blocks and simplifies the control flow graph
    TheFPM->addPass(SimplifyCFGPass());

    // Register the required analysis passes for transformation passes
    PassBuilder PB;
    PB.registerModuleAnalyses(*TheMAM);
    PB.registerFunctionAnalyses(*TheFAM);
    PB.crossRegisterProxies(*TheLAM, *TheFAM, *TheCGAM, *TheMAM);
}

/// Emission of Object File
bool EmitObjectFile(const std::string &Filename) {
    // Initialize the native target
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();

    // Detects the host triple automatically
    llvm::Triple TargetTriple(sys::getDefaultTargetTriple());
    TheModule->setTargetTriple(TargetTriple);

    // Searches the corresponding target to the triple in LLVM's register
    std::string Error;
    auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);
    if (!Target) {
        fprintf(stderr, "Error: Impossible to find the target: %s\n",
                Error.c_str());
        return false;
    }

    auto CPU = sys::getHostCPUName();
    auto Features = "";

    TargetOptions Opt;
    auto TheTargetMachine = Target->createTargetMachine(
        TargetTriple,
        CPU,
        Features,
        Opt,
        Reloc::PIC_ // Position Independent Code - required for linking
    );

    // Informs the data layout to the module
    TheModule->setDataLayout(TheTargetMachine->createDataLayout());

    // Opens the output file
    std::error_code EC;
    raw_fd_ostream Dest(Filename, EC, sys::fs::OF_None);
    if (EC) {
        fprintf(stderr, "Error: was not possible to open the file '%s': %s\n",
                Filename.c_str(), EC.message().c_str());
        return false;
    }

    // Uses legacy pass manager for object emission
    legacy::PassManager CodeGenPM;
    if (TheTargetMachine->addPassesToEmitFile(
            CodeGenPM, Dest, nullptr,
            CodeGenFileType::ObjectFile)) {
        fprintf(stderr, "Error: target does not support file object emission\n");
        return false;
    }

    // Runs the codegen pipeline and writes in the file
    CodeGenPM.run(*TheModule);
    Dest.flush();

    fprintf(stderr, "Object file generated: %s\n", Filename.c_str());
    return true;
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

        // Runs the otimization passes in the function
        TheFPM->run(*TheFunction, *TheFAM);

        return TheFunction;
    }

    TheFunction->eraseFromParent();
    return nullptr;
}

Value *IfExprAST::codegen() {
    // Generates the condition code
    Value *CondV = Cond->codegen();
    
    if (!CondV)
        return nullptr;

    // Converts the condition to a bool (i1) coparing it to zero
    if (CondV->getType()->isDoubleTy()) {
        CondV = Builder->CreateFCmpONE(
            CondV,
            ConstantFP::get(*TheContext, APFloat(0.0)),
            "ifcond");
    } else {
        CondV = Builder->CreateICmpNE(
            CondV,
            ConstantInt::get(*TheContext, APInt(64, 0, true)),
            "ifcond");
    }

    // Obtains the current function where the code is being inserted
    Function *TheFunction = Builder->GetInsertBlock()->getParent();

    // Creates three basic blocks:
    // ThenBB -> true branch code
    // ElseBB -> false branch code
    // MergeBB -> convergence point after if
    BasicBlock *ThenBB  = BasicBlock::Create(*TheContext, "then", TheFunction);
    BasicBlock *ElseBB  = BasicBlock::Create(*TheContext, "else");
    BasicBlock *MergeBB = BasicBlock::Create(*TheContext, "ifcont");

    // Generates the condition branch
    // if CondV is true -> jumps to ThenBB
    // if CondV is false -> jumps to ElseBB
    Builder->CreateCondBr(CondV, ThenBB, ElseBB);

    Builder->SetInsertPoint(ThenBB);
    Value *ThenV = Then->codegen();

    if (!ThenV)
        return nullptr;

    // Jumps to the merge block after finishing then
    Builder->CreateBr(MergeBB);

    // Refreshes ThenBB - the The codegen might have changed the current block
    // The phi node needs to knows which block the valu comes from
    ThenBB = Builder->GetInsertBlock();

    // Generates the Else Block
    // Inserts ElseBB in the function now that ThenBB is complete
    TheFunction->insert(TheFunction->end(), ElseBB);
    Builder->SetInsertPoint(ElseBB);

    Value *ElseV = Else->codegen();
    if (!ElseV)
        return nullptr;

    Builder->CreateBr(MergeBB);

    // Same reason, refreshes ElseBB after possible nesting
    ElseBB = Builder->GetInsertBlock();

    // Generates the Merge block with phi node
    TheFunction->insert(TheFunction->end(), MergeBB);
    Builder->SetInsertPoint(MergeBB);

    // If the branches types are distints, promotes Int to Double
    if (ThenV->getType()->isIntegerTy() && ElseV->getType()->isDoubleTy())
        ThenV = Builder->CreateSIToFP(
            ThenV, llvm::Type::getDoubleTy(*TheContext), "conv");
    else if (ThenV->getType()->isDoubleTy() && ElseV->getType()->isIntegerTy())
        ElseV = Builder->CreateSIToFP(
            ElseV, llvm::Type::getDoubleTy(*TheContext), "conv");

    // Phi node: chooses the correct value depending on which branch was executed
    // It is a central instruction of the SSA model for if/else
    PHINode *PN = Builder->CreatePHI(ThenV->getType(), 2, "iftmp");
    PN->addIncoming(ThenV, ThenBB);
    PN->addIncoming(ElseV, ElseBB);

    return PN;
}