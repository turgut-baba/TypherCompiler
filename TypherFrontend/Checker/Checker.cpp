#include "Checker.h"

namespace Checker {
    Checker::Checker(DiagnosticEngine &diags, MemoryAllocator *allocator)
        : diags_(diags), allocator_(allocator)
    {
        state_->analyzer_ = std::make_unique<SemanticAnalyzer>(diags_, allocator_);
        state_->type_checker_ = std::make_unique<TypeResolver>();
    }

    void Checker::StartChecker(SlabVector<AST::Statement*>& AST_tree)
    {
        std::cout << "Starting Checker..." << std::endl;
        state_->type_checker_->RegisterTypes(*state_->symbol_table_);
        
        state_->analyzer_->Analyze(*state_->AST_tree);
        std::cout << "Checker finished." << std::endl;
    }

}