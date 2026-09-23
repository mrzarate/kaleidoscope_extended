#include "sema/SymbolTable.h"
#include "llvm/Support/raw_ostream.h"

void SymbolTable::enterScope() {
    Scopes.push_back({});
}

void SymbolTable::exitScope() {
    if (Scopes.empty()) {
        llvm::errs() << "SymbolTable error: attempt to exit empty scope\n";
        return;
    }
    Scopes.pop_back();
}

bool SymbolTable::declare(const std::string &Name, ASTType VarType,
                          llvm::AllocaInst *Alloca) {
    if (Scopes.empty()) {
        llvm::errs() << "SymbolTable error: declaration out of scope\n";
        return false;
    }

    auto &CurrentScope = Scopes.back();

    if (CurrentScope.count(Name)) {
        llvm::errs() << "SymbolTable error: variable '" << Name
                     << "' already declared in this scope\n";
    }

    CurrentScope[Name] = SymbolInfo{VarType, Alloca};
    return true;
}

std::optional<SymbolInfo> SymbolTable::lookup(const std::string &Name) const {
    for (auto it = Scopes.rbegin(); it != Scopes.rend(); ++it) {
        auto found = it->find(Name);
        if (found != it->end()) 
            return found->second;
    }
    return std::nullopt;
}

bool SymbolTable::setAlloca(const std::string &Name,
                            llvm::AllocaInst *Alloca) {
    for (auto it = Scopes.rbegin(); it != Scopes.rend(); ++it) {
        auto found = it->find(Name);
        if (found != it->end()) {
            found->second.Alloca = Alloca;
            return true;
        }
    }
    llvm::errs() << "SymbolTable error: variable '" << Name
                 << "' not found\n";
    return false;
}