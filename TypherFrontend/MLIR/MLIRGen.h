#ifndef MLIR_GEN_H
#define MLIR_GEN_H

#include "Visitor.h"

#include "Function.h"
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
#include <fstream>

namespace MLIR {

	class Builder;
	class Emitter;

	class Generator: public AST::NodeVisitor {
	public:
		Generator(MemoryAllocator *allocator);
		~Generator();

		void BuildModule(SlabVector<AST::Statement*>& AST_tree);

		VISITOR

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

	private:
		MemoryAllocator *allocator_;
		Builder* builder_;
		Emitter* emit_;
	};
}

#endif
