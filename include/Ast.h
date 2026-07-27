#ifndef AST_H
#define AST_H

#include "llvm/IR/Value.h"
#include <string> // Strings general utility
#include <memory> // Smarter pointers and memory management
#include <vector> // Vector general utility

enum class NodeKind {
    NumberExpr,
    IntExpr,
    VariableExpr,
    BinaryExpr,
    CallExpr
};
/// Type - Represents the types of data supported by the language
/// Adds  new types such as Bool, Int, etc.
enum class Type {
    Double,
    Int,
    Unknown // Used before type-checking solver
};

/// ExprAST - Base class for all expression nodes.
class ExprAST {
    NodeKind Kind;
public:
    ExprAST(NodeKind Kind) : Kind(Kind) {}
    virtual ~ExprAST() = default;
    virtual llvm::Value *codegen() = 0;
    virtual Type getType() const = 0;
    NodeKind getKind() const { return Kind; }
};

/// NumberExprAST - Expression class for numeric literals like "1.0"
class NumberExprAST : public ExprAST {
    double Val;

public:
    NumberExprAST(double Val)
        : ExprAST(NodeKind::NumberExpr), Val(Val) {}
    double getVal() const { return Val; }
    Type getType() const override { return Type::Double; }
    llvm::Value *codegen() override;
};

//// IntExprAST - Expression class for integer literals like "12"
class IntExprAST : public ExprAST {
    long long Num;

public:
    IntExprAST(long long Num)
        : ExprAST(NodeKind::IntExpr), Num(Num) {}
    long long getVal() const { return Num; }
    Type getType() const override { return Type::Int; }  
    llvm::Value *codegen() override; 
};

/// VariableExprAST - Expression class for referencing a variable, like "a"
class VariableExprAST : public ExprAST {
    std::string Name;
    Type VarType;

public:
    VariableExprAST(const std::string &Name, Type VarType = Type::Unknown) 
        : ExprAST(NodeKind::VariableExpr), Name(Name), VarType(VarType) {}
    const std::string &getName() const { return Name; }
    Type getType() const override { return VarType; }
    void setType(Type T) { VarType = T; }
    llvm::Value *codegen() override;
};

/// BinaryExprAST - Expression class for a binary operator
class BinaryExprAST : public ExprAST {
    char Op;
    std::unique_ptr<ExprAST> LHS, RHS;
    Type ResultType;

public:
    BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS,
                std::unique_ptr<ExprAST> RHS)
        : ExprAST(NodeKind::BinaryExpr), Op(Op),
          LHS(std::move(LHS)), RHS(std::move(RHS)),
          ResultType(Type::Unknown) {}

    char getOp() const { return Op; }
    ExprAST *getLHS() const { return LHS.get(); }
    ExprAST *getRHS() const { return RHS.get(); }

    Type getType() const override { return ResultType; }
    void setType(Type T) { ResultType = T; }
    llvm::Value *codegen() override;
};

/// CallExprAST - Expression class for function calls
class CallExprAST: public ExprAST {
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;
    Type ReturnType;

public:
    CallExprAST(const std::string &Callee, 
                std::vector<std::unique_ptr<ExprAST>> Args)
        : ExprAST(NodeKind::CallExpr), Callee(Callee),
        Args(std::move(Args)), ReturnType(Type::Unknown) {}

    const std::string &getCallee() const { return Callee; }
    Type getType() const override { return ReturnType; }
    void setType(Type T) { ReturnType = T; }
    llvm::Value *codegen() override;
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name and its argument names (thus implicitly the number
/// of arguments the function takes)
class PrototypeAST {
    std::string Name;
    std::vector<std::string> Args;
    Type ReturnType;

public:
    PrototypeAST(const std::string &Name, std::vector<std::string> Args,
                    Type ReturnType = Type::Double)
        : Name(Name), Args(std::move(Args)), ReturnType(ReturnType) {}

    const std::string &getName() const { return Name; }
    const std::vector<std::string> &getArgs() const { return Args; }
    Type getReturnType() const { return ReturnType; }
    llvm::Function *codegen();
};

/// FunctionAST - This class represents a function definition itself
class FunctionAST {
    std::unique_ptr<PrototypeAST> Proto;
    std::unique_ptr<ExprAST> Body;

public:
    FunctionAST(std::unique_ptr<PrototypeAST> Proto, 
                std::unique_ptr<ExprAST> Body)
        : Proto(std::move(Proto)), Body(std::move(Body)) {}

    ExprAST *getBody() const { return Body.get(); }
    llvm::Function *codegen();
};

#endif 