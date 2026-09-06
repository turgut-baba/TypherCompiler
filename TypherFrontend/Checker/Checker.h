#ifndef CHECKER_H
#define CHECKER_H

#include "Parser.h"
#include "SemanticAnalysis/SemanticAnalyzer.h"
#include "SemanticAnalysis/TypeResolver.h"
#include "Memory/MemAlloc.h"
#include "Memory/BumpPtrAlloc.h"

#include "CheckerState.h"

namespace Checker {

	class Type;

	class Checker {
	public:
		explicit Checker(DiagnosticEngine &diags, MemoryAllocator *allocator);

		void StartChecker(SlabVector<AST::Statement*>& AST_tree);

		template <AllocatorType Type = AllocatorType::SLAB>
		auto Allocator() -> typename AllocTypeMap<Type>::type*
		{
			if constexpr (Type == AllocatorType::DTOR)
				return allocator_->dtorAlloc.get();
			if constexpr (Type == AllocatorType::SLAB)
				return allocator_->slabAlloc.get();
			if constexpr (Type == AllocatorType::BUMP)
				return allocator_->bumpAlloc.get();
		}

		SemanticAnalyzer* GetAnalyzer() const
		{
			return state_->analyzer_.get();
		}

		SymbolTable* Table()
		{
			return state_->symbol_table_.get();
		}

		void check_module(SlabVector<AST::Statement*> ASTTree);
	private:
		void CheckNode(AST::ASTNode* node);

		DiagnosticEngine &diags_;
		MemoryAllocator* allocator_;

		CheckerState* state_;
	};
}

#endif