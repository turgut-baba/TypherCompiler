#ifndef CHEKER_STATE_H
#define CHEKER_STATE_H

#include <memory>
#include "Memory/MemAlloc.h"
#include "Log/Diagnostics.h"

namespace Checker {

	class SymbolTable;
	class SemanticAnalyzer;
	class ControlFlowGraph;
	class TypeResolver;

	struct CheckerState {
		std::unique_ptr<SymbolTable> symbol_table_;

		std::unique_ptr<TypeResolver> type_checker_;
		std::unique_ptr<SemanticAnalyzer> analyzer_;
		std::unique_ptr<ControlFlowGraph> cfg_;
		std::unique_ptr<SlabVector<AST::Statement*>> AST_tree;
	};
}

#endif