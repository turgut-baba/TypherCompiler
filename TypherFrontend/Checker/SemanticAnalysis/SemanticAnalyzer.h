#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "Visitor.h"
#include "Expression.h"
#include "Expressions/Operator.h"
#include "Literals/IntegerLiteral.h"

#include "Log/Diagnostics.h"
#include "Memory/MemAlloc.h"

#include "Function.h"
#include "Visitor.h"
#include "Expressions/Operator.h"
#include "Expressions/CallExpression.h"
#include "Expressions/MemoryOperation.h"
#include "Expressions/InitializerList.h"
#include "statements/ReturnStatement.h"
#include "statements/IfStatement.h"
#include "statements/WhileStatement.h"
#include "statements/ForStatement.h"
#include "Literals/IntegerLiteral.h"
#include "Literals/StringLiteral.h"
#include "statements/ExpressionStatement.h"

#include "SymbolTable.h"

namespace Checker {
    class SemanticAnalyzer: public AST::NodeVisitor {
    public:
        SemanticAnalyzer(DiagnosticEngine &diags, MemoryAllocator *allocator);
        ~SemanticAnalyzer() = default;
        
        VISITOR
        
        std::unique_ptr<SymbolTable>& Analyze(SlabVector<AST::Statement*>& AST_tree);
    private:
        std::unique_ptr<SymbolTable> symbol_table_;

        DiagnosticEngine &diags_;
		MemoryAllocator* allocator_;
    };
}

#endif