#ifndef TYPE_RESOLVER_H
#define TYPE_RESOLVER_H

#include "SymbolTable.h"

namespace Checker {

class TypeResolver
{
public:
    TypeResolver() = default;

    void RegisterTypes(SymbolTable& symbol_table);

    void CheckTypes(SymbolTable& symbol_table);
private:
    void CheckBuiltinType(const std::shared_ptr<Type>& type);
    void CheckUserDefinedType(const std::shared_ptr<Type>& type);
};

}

#endif // TYPE_CHECKER_H