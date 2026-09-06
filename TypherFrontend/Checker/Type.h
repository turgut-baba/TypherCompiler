#ifndef TYPE_H
#define TYPE_H

#include "ASTNode.h"

namespace Checker {
    enum TypeKind {
        UNDEFINED,
        BUILT_IN,
        USER_DEFINED,
    };

    enum class TypeFlags {
        CONST = 1 << 0,
        VOLATILE = 1 << 1,
        POINTER = 1 << 2,
        ARRAY = 1 << 3,
        FUNCTION = 1 << 4
    };

    inline TypeFlags& operator|=(TypeFlags& a, TypeFlags b)
    {
        return a = static_cast<TypeFlags>(
            static_cast<std::underlying_type_t<TypeFlags>>(a) | 
            static_cast<std::underlying_type_t<TypeFlags>>(b)
        );
    }

    inline TypeFlags& operator&=(TypeFlags& a, TypeFlags b)
    {
        return a = static_cast<TypeFlags>(
            static_cast<std::underlying_type_t<TypeFlags>>(a) & 
            static_cast<std::underlying_type_t<TypeFlags>>(b)
        );
    }

    inline TypeFlags operator~(TypeFlags a)
    {
        return static_cast<TypeFlags>(~static_cast<int>(a));
    }

    inline TypeFlags operator&(TypeFlags a, TypeFlags b)
    {
        return static_cast<TypeFlags>(
            static_cast<int>(a) & static_cast<int>(b)
        );
    }

    inline bool operator!=(TypeFlags a, int b)
    {
        return static_cast<int>(a) != static_cast<int>(b);
    }

    inline bool operator==(TypeFlags a, int b)
    {
        return static_cast<int>(a) == static_cast<int>(b);
    }
    
    class Type {
    public:
        Type() = delete;
        Type(AstBuiltinTypes kind); // For BUILT_IN
        Type(std::string name, int size); // For USER_DEFINED

        TypeKind Kind() const 
        { 
            return kind_;
        }

        bool IsSameAs(const Type& other) const 
        {
            if(kind_ == TypeKind::USER_DEFINED && other.kind_ == TypeKind::USER_DEFINED) {
                return name_ == other.name_;
            }

            return kind_ == other.kind_ && size_in_bytes == other.size_in_bytes;
        }

        TypeFlags GetFlags() const { return flags_; }

        void SetFlag(TypeFlags flag) { flags_ |= flag; }
        void ClearFlag(TypeFlags flag) { flags_ &= ~flag; }
        bool HasFlag(TypeFlags flag) const { return (flags_ & flag) != 0; }

        static std::shared_ptr<Type> MakeVoid() { return std::make_shared<Type>(AstBuiltinTypes::VOID); }
        static std::shared_ptr<Type> MakeByte() { return std::make_shared<Type>(AstBuiltinTypes::BOOL); }
        static std::shared_ptr<Type> MakeChar() { return std::make_shared<Type>(AstBuiltinTypes::CHAR); }
        static std::shared_ptr<Type> MakeShort() { return std::make_shared<Type>(AstBuiltinTypes::SHORT); }
        static std::shared_ptr<Type> MakeInt() { return std::make_shared<Type>(AstBuiltinTypes::INT); }
        static std::shared_ptr<Type> MakeLong() { return std::make_shared<Type>(AstBuiltinTypes::LONG); }
        static std::shared_ptr<Type> MakeFloat() { return std::make_shared<Type>(AstBuiltinTypes::FLOAT); }
        static std::shared_ptr<Type> MakeDouble() { return std::make_shared<Type>(AstBuiltinTypes::DOUBLE); }
    private:
        int size_in_bytes;
        std::string name_ = "";  // For USER_DEFINED
        AstBuiltinTypes builtin_ = AstBuiltinTypes::NONE; // For BUILT_IN
        SlabVector<std::shared_ptr<Type>> param_types;  // For FUNCTION
        TypeKind kind_;
        TypeFlags flags_;
    };
}

#endif