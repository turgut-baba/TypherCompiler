#include "Type.h"

namespace Checker {
    Type::Type(AstBuiltinTypes kind)
    {
        kind_ = BUILT_IN;
        switch (kind)
        {
        case AstBuiltinTypes::VOID:
            name_ = "void";
            size_in_bytes = 0;
            break;
        case AstBuiltinTypes::BOOL:
            name_ = "bool";
            size_in_bytes = 1;
            break;  
        case AstBuiltinTypes::CHAR:
            name_ = "char";
            size_in_bytes = 1;
            break;
        case AstBuiltinTypes::SHORT:
            name_ = "short";
            size_in_bytes = 2;
            break;
        case AstBuiltinTypes::INT:
            name_ = "int";
            size_in_bytes = 4;
            break;
        case AstBuiltinTypes::LONG:
            name_ = "long";
            size_in_bytes = 8;
            break;
        case AstBuiltinTypes::FLOAT:
            name_ = "float";
            size_in_bytes = 4;
            break;
        case AstBuiltinTypes::DOUBLE:
            name_ = "double";
            size_in_bytes = 8;
            break;
        default:
            break;
        }
    }

    Type::Type(std::string name, int size)
    {
        name_ = name;
        size_in_bytes = size;
        kind_ = USER_DEFINED;
    }
}