#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "Ast.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Instructions.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

// SymbolInfo - Informations about a declared variable
struct SymbolInfo {
    Type VarType; // variable type
    llvm::AllocaInst *Alloca // pointer to alloca in LLVM stack
                             // nullptr until codegen runs
};

/// SymbolTable - Symbol Table with support to nested scopes
///
/// Organized as a map stack - each level of the stack
/// represents a scope (function, block, loop, etc.):
///
///     scope 0 (function) -> parameters
///     scope 1 (block)    -> local variables of the block
///     scope 2 (if)       -> block variables of the if
///     ...
///
///     Lookup travels from the top of the stack to the base
///     intern variable shadows external variables with the same name
class SymbolTable {
    std::vector<std::unordered_map<std::string, SymbolInfo>> Scopes;

public:
    /// Enters in a new scope (ex: upon entering a block {})
    void enterScope();

    /// Exit the current scope (ex: upon closing a block })
    void exitScope();

    /// Declares a variable in the current scope
    /// Returns false if already exits a variable with this name
    /// in the current scope (returning an error)
    bool declare(const std::string &Name, Type VarType,
                 llvm::AllocaInst *Alloca = nullptr);

    /// Searchs variable by the name, from the internal scope
    /// to the most external. Return nullptr if not found
    std::optional<SymbolInfo> lookup(const std::string &Name) const;

    /// Refreshes the AllocaInst of a variable already allocated
    /// Used by the codegen after generating alloca.
    bool setAlloca(const std::string &Name, llvm::AllocaInst *Alloca);

    /// Return true if there is at least one open scope
    bool hasScope() const { return !Scopes.empty(); }

    /// Return the number of active scopes (useful for debug)
    size_t depth() const { return Scopes.size(); }
};

#endif // SYMBOLTABLE_H