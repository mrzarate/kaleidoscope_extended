#ifndef AST_H
#define AST_H

#include "llvm/IR/Value.h"
#include "llvm/IR/Function.h"
#include <string> // Strings general utility
#include <memory> // Smarter pointers and memory management
#include <vector> // Vector general utility

enum class NodeKind {
    NumberExpr,
    IntExpr,
    VariableExpr,
    BinaryExpr,
    CallExpr,
    IfExpr,
    VarDeclExpr,
    AssignExpr,
    BlockExpr,
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

/// IfExprAST - Expression class for if/else, both branches must
/// return a value, similar to C operator
class IfExprAST : public ExprAST {
    std::unique_ptr<ExprAST> Cond; // Condition expression
    std::unique_ptr<ExprAST> Then; // if body
    std::unique_ptr<ExprAST> Else; // else boldy
    Type ResultType;

public:
    IfExprAST(std::unique_ptr<ExprAST> Cond,
              std::unique_ptr<ExprAST> Then,
              std::unique_ptr<ExprAST> Else)
        :   ExprAST(NodeKind::IfExpr),
            Cond(std::move(Cond)),
            Then(std::move(Then)),
            Else(std::move(Else)),
            ResultType(Type::Unknown) {}

    ExprAST *getCond() const { return Cond.get(); }
    ExprAST *getThen() const { return Then.get(); }
    ExprAST *getElse() const { return Else.get(); }
    Type getType() const override { return ResultType; }
    void setType(Type T) { ResultType = T; }
    llvm::Value *codegen() override;
};
/// VarDeclExprAST - Declaration of local variable
/// Sintax: double x = expr; or int y = expr;
class VarDeclExprAST : public ExprAST {
    std::string Name;
    Type VarType;
    std::unique_ptr<ExprAST> Init;
public:
    VarDeclExprAST(const std::string &Name, Type VarType,
                std::unique_ptr<ExprAST> Init)
        : ExprAST(NodeKind::VarDeclExpr), Name(Name),
          VarType(VarType), Init(std::move(Init)) {}
    const std::string &getName() const { return Name; }
    Type getType() const override { return VarType; }
    ExprAST *getInit() const { return Init.get(); }
    llvm::Value *codegen() override;

};

/// AssignExprAST - Assignment of an existing variable
/// Sintax: x = expr;
class AssignExprAST : public ExprAST {
    std::string Name; // target variable name
    std::unique_ptr<ExprAST> Value; // new value
    Type VarType; // type of the variable
public:
    AssignExprAST(const std::string &Name,
                  std::unique_ptr<ExprAST> Value)
        : ExprAST(NodeKind::AssignExpr), Name(Name),
          Value(std::move(Value)), VarType(Type::Unknown) {}
    const std::string &getName() const { return Name; }
    ExprAST *getValue() const { return Value.get(); }
    Type getType() const override { return VarType; }
    void setType(Type T) { VarType = T;}
    llvm::Value *codegen() override;
};

/// BlockExprAST - Block of statements
/// Sintax: { stmt1; stmt2; expr }
/// The block value is the value of the last expression
class BlockExprAST : public ExprAST {
    std::vector<std::unique_ptr<ExprAST>> Stmts;
    Type ResultType;
public:
    BlockExprAST(std::vector<std::unique_ptr<ExprAST>> Stmts)
        : ExprAST(NodeKind::BlockExpr), Stmts(std::move(Stmts)),
        ResultType(Type::Unknown) {}
    const std::vector<std::unique_ptr<ExprAST>> &getStmts() const {
        return Stmts;
    }
    Type getType() const override { return ResultType; }
    void setType(Type T) { ResultType = T; }
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