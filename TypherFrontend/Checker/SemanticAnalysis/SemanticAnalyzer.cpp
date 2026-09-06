#include "SemanticAnalyzer.h"

namespace Checker {

    SemanticAnalyzer::SemanticAnalyzer(DiagnosticEngine &diags, MemoryAllocator *allocator)
        : symbol_table_(std::make_unique<SymbolTable>()), diags_(diags), allocator_(allocator)    
    {

    }

    std::unique_ptr<SymbolTable>& SemanticAnalyzer::Analyze(SlabVector<AST::Statement*>& AST_tree)
    {
        for (AST::ASTNode* node : AST_tree) {
            if (node != nullptr) {
                node->Accept(this);
            }
        }

        return symbol_table_;
    }

    void SemanticAnalyzer::Visit(AST::VariableDeclaration *node)
    {
        AstBuiltinTypes type = node->Type();

        for(auto declarator: node->Declarators()) {
            AST::Expression* rhs = declarator->Expr();
            rhs->Accept(this);

            Symbol sym;
            sym.name = declarator->Ident()->Value();
            sym.kind = SymbolKind::VARIABLE;
            sym.type = std::make_shared<Type>(node->Type());
            sym.is_defined = declarator->IsDefinition();

            symbol_table_->Declare(declarator->Ident()->Value(), sym);
        }
    }

    void SemanticAnalyzer::Visit(AST::Identifier *node)
    {
        bool found = symbol_table_->LookupName(node->Value());
        if(!found) {
            diags_.report<DiagLevel::Error>({}) 
				<< "Undefined identifier: " << node->Value();
        }
    }


    void SemanticAnalyzer::Visit(AST::Function* node) {}
    void SemanticAnalyzer::Visit(AST::Statement* node) {}
    void SemanticAnalyzer::Visit(AST::VariableDeclarator* node) {}
    void SemanticAnalyzer::Visit(AST::Expression* node) {}
    void SemanticAnalyzer::Visit(AST::IntegerLiteral* node) {}
    void SemanticAnalyzer::Visit(AST::StringLiteral* node) {}
    void SemanticAnalyzer::Visit(AST::Operator* node) {}
    void SemanticAnalyzer::Visit(AST::CallExpression* node) {}
    void SemanticAnalyzer::Visit(AST::MemoryOperation* node) {}
    void SemanticAnalyzer::Visit(AST::ReturnStatement* node) {}
    void SemanticAnalyzer::Visit(AST::IfStatement* node) {}
    void SemanticAnalyzer::Visit(AST::ExpressionStatement* node) {}
    void SemanticAnalyzer::Visit(AST::WhileStatement* node) {}
    void SemanticAnalyzer::Visit(AST::ForStatement* node) {}
    void SemanticAnalyzer::Visit(AST::InitializerList* node) {}

    
}
