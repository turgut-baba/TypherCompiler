#include "MLIRBuilder.h"
#include "MLIRGen.h"

namespace MLIR {

    Builder::Builder(Generator* gen) 
		:gen_(gen)
    {
        mlir::func::registerAllExtensions(registry);
		mlir::LLVM::registerInlinerInterface(registry);

		registry.insert<mlir::arith::ArithDialect>();

		context = std::make_shared<mlir::MLIRContext>(registry);

		context->loadDialect<mlir::arith::ArithDialect>();
    }

    void Builder::Build(SlabVector<AST::Statement*>& ASTTree)
	{
        context->getOrLoadDialect<mlir::typher::TypherDialect>();

		op = std::make_shared<mlir::OpBuilder>(context.get());
		
		theModule = mlir::ModuleOp::create(op->getUnknownLoc());

		// TODO: Add global (modular) context.
    	
		//llvm::ScopedHashTableScope<llvm::StringRef, mlir::Value> varScope(symbolTable);

		for (AST::ASTNode* node : ASTTree)
      		node->Accept(gen_);

		if (failed(mlir::verify(theModule))) {
			theModule.emitError("module verification error");
			return;
		}

        theModule->dump();
	}

	void Builder::GenBody(AST::Body* body, mlir::Location& location) 
	{
		llvm::ScopedHashTableScope<llvm::StringRef, mlir::Value> varScope(symbolTable);
		
		for (AST::ASTNode* child: body->Statements()) {
			child->Accept(gen_);
			// if (SOME ERROR HANDLING) { function.erase(); return; }
    	}
		
		if (op->getBlock()->empty() || 
			!op->getBlock()->back().hasTrait<mlir::OpTrait::IsTerminator>()) {
			mlir::typher::YieldOp::create(*op, location);
		}
	}

    mlir::Value Builder::LvalueToRvalue(mlir::Value addr, mlir::Location location)
	{
		mlir::Type addrType = addr.getType();
		
		if (auto memrefType = mlir::dyn_cast<mlir::MemRefType>(addrType)) {
			mlir::Type elementType = memrefType.getElementType();
			return mlir::typher::LoadOp::create(*op, location, elementType, addr);
		} 

		if (auto ptrType = mlir::dyn_cast<mlir::typher::PointerType>(addrType)) {
        	mlir::Type elementType = ptrType.getElementType();
        	return op->create<mlir::typher::LoadOp>(location, elementType, addr);
    	}

		if(auto arrayType = mlir::dyn_cast<mlir::typher::ArrayType>(addrType)) {
			// TODO: This doesn't work because the array variables are not allocated as ArrayType.
			mlir::Type elementType = arrayType.getElementType();
			return op->create<mlir::typher::LoadOp>(location, elementType, addr);
		}

		return addr;
	}

	mlir::Value Builder::GenArrayAccess(AST::MemoryOperation* node, mlir::Type array_type)
	{
		llvm::SmallVector<mlir::Value, 4> indexValues;
		mlir::StringAttr persistentName = op->getStringAttr(
			((AST::Identifier*)node->GetExpression())->Value() // TODO: Find a better approach to this.
		);
		
		mlir::Value address = symbolTable.lookup(persistentName.getValue());
		// Check if base address points to an ArrayType (!typher.ptr<!typher.array<...>>)
		auto ptrType = mlir::dyn_cast<mlir::typher::PointerType>(address.getType());
		if (ptrType && mlir::isa<mlir::typher::ArrayType>(ptrType.getElementType())) {
			// Prepend index 0 to dereference the array pointer
			mlir::Value zeroIdx = op->create<mlir::arith::ConstantIndexOp>(loc(node->Loc()), 0);
			indexValues.push_back(zeroIdx);
		}

		for (AST::Expression* indexExpr : node->ArrayIndices()) {
			indexExpr->Accept(gen_);
			mlir::Value indexVal = retValue;
			
			if (!indexVal.getType().isIndex()) {
				indexVal = op->create<mlir::arith::IndexCastOp>(
					loc(indexExpr->Loc()), 
					op->getIndexType(), 
					indexVal
				);
			}
			indexValues.push_back(indexVal);
		}

		auto resultPtrType = mlir::typher::PointerType::get(op->getContext(), array_type);

		// address is guaranteed to be !typher.ptr<!typher.array<3 x i32>>
		address = op->create<mlir::typher::AccessOp>(
			loc(node->Loc()),
			resultPtrType, // !typher.ptr<i32>
			address,       // !typher.ptr<!typher.array<3 x i32>>
			indexValues    // [%c0, %c1]
		);

		return address;
	}
    
}

