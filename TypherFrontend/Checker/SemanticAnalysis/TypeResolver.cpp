#include "TypeResolver.h"

namespace Checker {
    void TypeResolver::RegisterTypes(SymbolTable& symbol_table)
    {
        // Register built-in types
        symbol_table.Declare("void", Symbol{"void", SymbolKind::TYPE_DEF, Type::MakeVoid()});
        symbol_table.Declare("byte", Symbol{"byte", SymbolKind::TYPE_DEF, Type::MakeByte()});
        symbol_table.Declare("char", Symbol{"char", SymbolKind::TYPE_DEF, Type::MakeChar()});
        symbol_table.Declare("short", Symbol{"short", SymbolKind::TYPE_DEF, Type::MakeShort()});
        symbol_table.Declare("int", Symbol{"int", SymbolKind::TYPE_DEF, Type::MakeInt()});
        symbol_table.Declare("float", Symbol{"float", SymbolKind::TYPE_DEF, Type::MakeFloat()});
        symbol_table.Declare("double", Symbol{"double", SymbolKind::TYPE_DEF, Type::MakeDouble()});

        //AST::IterateTree(*state_->AST_tree, [&](AST::ASTNode* node) {
        //    // TODO: We will check user defined types here. For example, if we encounter a struct or class declaration, we can register it in the symbol table.
        //});
    }

}