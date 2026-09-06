#include "MLIRGen.h"
#include "MLIRBuilder.h"
#include "MLIREmitter.h"
#include "MLIRHelpers.h"

namespace MLIR {
	using llvm::dyn_cast;
	using llvm::SmallVector;

	Generator::Generator(MemoryAllocator *allocator)
		:allocator_(allocator)
	{
		// TODO: fix this
		builder_ = Allocator()->Allocate<Builder>(this);
		emit_ = Allocator()->Allocate<Emitter>();
	}

	Generator::~Generator()
	{

	}

	void Generator::BuildModule(SlabVector<AST::Statement*>& AST_tree)
    {
        builder_->Build(AST_tree);
        emit_->Emit(builder_->Context(), builder_->Module());
    }

	void Generator::Visit(AST::ExpressionStatement* node)
	{
		node->Expr()->Accept(this);
	}

	void Generator::Visit(AST::Function* node)
	{
    	llvm::ScopedHashTableScope<llvm::StringRef, mlir::Value> varScope(builder_->symbolTable);

		builder_->op->setInsertionPointToEnd(builder_->Module().getBody());

		auto location = builder_->loc(node->Loc());
		
		mlir::Type returnType = ASTTypeToMlirType(node->ReturnType(), builder_->op);
		ApplyTypeModifiers(node->Declarator(), returnType, builder_->op);

		auto funcArgs = node->Params();

		llvm::SmallVector<mlir::Type, 4> argTypes;

		for (size_t i = 0; i < funcArgs.size(); i++) 
		{
			auto arg_type = ASTTypeToMlirType(funcArgs[i].Type(), builder_->op);
			ApplyTypeModifiers(funcArgs[i].AsDeclarator(), arg_type, builder_->op);
			
			argTypes.push_back(arg_type);
		}

		mlir::FunctionType funcType = builder_->op->getFunctionType(argTypes, returnType);
		mlir::typher::FuncOp function = mlir::typher::FuncOp::create(*builder_->op, location, node->Name(),
										funcType);

		mlir::Type expectedType = returnType;

		mlir::Block &entryBlock = function.front();
		auto entryArgs = entryBlock.getArguments();

		builder_->op->setInsertionPointToStart(&entryBlock);

		for (size_t i = 0; i < funcArgs.size(); ++i) {
			mlir::StringAttr paramName = builder_->op->getStringAttr(funcArgs[i].Name());
			mlir::Value mlirArg = entryArgs[i]; // The incoming Rvalue argument
			
			// 1. Get the type of the argument
			mlir::Type argType = mlirArg.getType();

			// 2. Allocate a local stack slot for the parameter (Lvalue)
			// Note: If using MemRefType for stack slots, build memref<argType>
			// auto memrefType = mlir::MemRefType::get({}, argType);
			auto memrefType = mlir::typher::PointerType::get(builder_->op->getContext(), argType);
			mlir::Value paramAlloc = builder_->op->create<mlir::typher::AllocaOp>(location, memrefType);

			// 3. Store the incoming argument value into the stack slot
			builder_->op->create<mlir::typher::AssignOp>(location, mlirArg, paramAlloc);

			// 4. Register the STACK SLOT (Lvalue) in the symbol table, NOT the raw argument
			builder_->symbolTable.insert(paramName.getValue(), paramAlloc);
		}

		builder_->GenBody(node->GetBody(), location);

 		mlir::typher::ReturnOp returnOp;
		if (!entryBlock.empty()) {
			returnOp = dyn_cast<mlir::typher::ReturnOp>(entryBlock.back());
		}

		if (!returnOp) {
			// TODO: this place causes a seg fault.
			mlir::typher::ReturnOp::create(*builder_->op, location);
		} else if(returnOp.hasOperand()){
			function.setType(builder_->op->getFunctionType(
          		function.getFunctionType().getInputs(), returnType));
		}

		// retType = function;
	}

	void Generator::Visit(AST::Statement* node)
	{
		UNREACHABLE("Pure statment reached on MLIRGen.");
	}

	void Generator::Visit(AST::VariableDeclarator* node) 
	{
		auto location = builder_->loc(node->Loc());

		mlir::Type varType = ASTTypeToMlirType(((AST::VariableDeclaration*)node->Parent())->Type(), builder_->op);

		ApplyTypeModifiers(node, varType, builder_->op);
		
		auto memrefType = mlir::typher::PointerType::get(builder_->op->getContext(), varType);
		//auto memrefType = mlir::MemRefType::get({}, varType);

		mlir::Value address = mlir::typher::AllocaOp::create(*builder_->op, 
			location, memrefType);

		if (node->Expr()) {
			builder_->retValue = address;
			node->Expr()->Accept(this);
			mlir::Value initialValue = builder_->LvalueToRvalue(builder_->retValue, location);
			mlir::Type addrType = initialValue.getType();
			
			mlir::typher::AssignOp::create(*builder_->op, location, initialValue, address);
		}
		
		mlir::StringAttr varName = builder_->op->getStringAttr(node->Name());
		builder_->symbolTable.insert(varName.getValue(), address);

		builder_->retValue = address;
	}
	
	void Generator::Visit(AST::VariableDeclaration* node) 
	{
		auto decl_list = node->Declarators();
		for(int i = 0; i < decl_list.size(); i++)
		{
			decl_list[i]->Accept(this);
			if (failed(builder_->declare(decl_list[i]->Name(), builder_->retValue)))
				return;
		}
	}
	
	void Generator::Visit(AST::Expression* node) 
	{
		node->Accept(this);
	}

	void Generator::Visit(AST::CallExpression* node) 
	{
		std::string callee = node->Callee();
		auto location = builder_->loc(node->Loc());
		// Codegen the operands first.
		SmallVector<mlir::Value, 4> operands;
		for (auto &expr : node->Args()) {
			expr->Accept(this);
			operands.push_back(builder_->retValue);
		}

		mlir::Type varType = builder_->op->getI32Type();//ASTTypeToMlirType(node->Type(), builder_->op);

		// TODO: do this better.
		builder_->retValue = (mlir::Value)mlir::typher::GenericCallOp::create(
				*builder_->op, 
				location, 
				varType, // Added parameter
				callee, 
				operands
			).getResult(0);
	}

	void Generator::Visit(AST::ReturnStatement* node) 
	{
		auto location = builder_->loc(node->Loc());

		// 'return' takes an optional expression, handle that case here.
		if (node->Expr() != nullptr) {
			node->Expr()->Accept(this);

			mlir::Value addr = builder_->retValue; // This is the memref<i32> from builder_->symbolTable
			addr = builder_->LvalueToRvalue(addr, location);

			mlir::typher::ReturnOp::create(*builder_->op, location, addr);
		} else {
			mlir::typher::ReturnOp::create(*builder_->op, location,
				builder_->retValue);
		}
	}

	void Generator::Visit(AST::InitializerList* node)
	{
		auto location = builder_->loc(node->Loc());

		const auto &elements = node->GetElements();
		size_t count = elements.size();

		if (count == 0) {
			// TODO: Handle empty initializer list {} if supported
			return;
		}

		mlir::Type elemType = builder_->op->getI32Type(); 

		mlir::Value tempAlloc = builder_->retValue;

		// 4. Element pointer type for GEP: !typher.ptr<T>
		auto elemPtrType = mlir::typher::PointerType::get(
			builder_->op->getContext(), elemType
		);

		// Constant index 0 needed for outer pointer dereference
		mlir::Value zeroIdx = builder_->op->create<mlir::arith::ConstantIndexOp>(
			location, 0
		);

		// 5. Unified Loop: Evaluate and store all elements [0..N-1]
		for (size_t i = 0; i < count; ++i) {
			// Evaluate element expression -> sets builder_->retValue
			elements[i]->Accept(this);
			mlir::Value valToStore = builder_->retValue;

			llvm::SmallVector<mlir::Value, 4> indexValues;

			mlir::Value elemIdx = builder_->op->create<mlir::arith::ConstantIndexOp>(
				location, i
			);

			indexValues.push_back(zeroIdx);
			indexValues.push_back(elemIdx);

			// ✅ FIX: Pass TWO indices {%c0, %i} so AccessOp dereferences 
			// !typher.ptr<!typher.array<N x T>> to !typher.ptr<T>
			mlir::Value elemPtr = builder_->op->create<mlir::typher::AccessOp>(
				location, 
				elemPtrType, 
				tempAlloc, 
				indexValues // <--- Prepend zeroIdx here!
			);

			// Store value into element address
			builder_->op->create<mlir::typher::StoreOp>(
				location, valToStore, elemPtr
			);
		}

		// Return pointer to temporary array aggregate
		builder_->retValue = tempAlloc;
	}


	void Generator::Visit(AST::MemoryOperation* node) 
	{
		//node->GetExpression()->Accept(this);
		mlir::Value address = builder_->retValue;
		
		for (int i = 0; i < node->AddressDepth(); i++) {
			mlir::StringAttr persistentName = builder_->op->getStringAttr(
				((AST::Identifier*)node->GetExpression())->Value() // TODO: Find a better approach to this.
			);
			address = builder_->symbolTable.lookup(persistentName.getValue());
			
			if (!address) {
				emitError(builder_->loc(node->Loc()), "Undefined variable target for address-of operator");
			}
		}

		if (!node->ArrayIndices().empty()) 
		{
			mlir::Type mlirElemType = builder_->op->getI32Type(); // TODO: change this.

			address = builder_->GenArrayAccess(node, mlirElemType);

			//address = builder_->LvalueToRvalue(address, builder_->loc(node->Loc()));
		}

		for (int i = 0; i < node->DeRefDepth(); i++) { 
			std::cout << "Deref depth: " << i << std::endl;
			address = builder_->LvalueToRvalue(address, builder_->loc(node->Loc()));
		}

		builder_->retValue = address;
	}

	void Generator::Visit(AST::Identifier* node) 
	{
		auto location = builder_->loc(node->Loc());

		if (auto variable = builder_->symbolTable.lookup(node->Value()))
		{
			mlir::Type varType = variable.getType();

			builder_->retValue = variable; 
			return;
		}

		// TODO: log error
		return;
	}

	
	void Generator::Visit(AST::IntegerLiteral* node) 
	{
		if(node->IsFloating()) {
			builder_->retValue = mlir::typher::ConstantOp::create(*builder_->op,
			 builder_->loc(node->Loc()), builder_->op->getF32Type(), node->Value<double>());
		} else {
			builder_->retValue = mlir::typher::ConstantOp::create(*builder_->op,
			 builder_->loc(node->Loc()), builder_->op->getI32Type(), node->Value<int>());
		}
	}

	void Generator::Visit(AST::StringLiteral* node) 
	{
		if(node->IsChar()) {
			builder_->retValue = mlir::typher::ConstantOp::create(*builder_->op,
			 	builder_->loc(node->Loc()), builder_->op->getI8Type(), node->Value<char>());
		} else {
			
		}
	}

	void Generator::Visit(AST::ForStatement* node) 
	{
		auto location = builder_->loc(node->Loc()); 

		if (node->InitializeStmt()) {
			node->InitializeStmt()->Accept(this);
		}

		auto whileOp = mlir::typher::WhileOp::create(*builder_->op, location);
		
		mlir::Block* condBlock = builder_->op->createBlock(&whileOp.getCondRegion());
		builder_->op->setInsertionPointToStart(condBlock);
		
		if (node->ConditionExpr()) {
			node->ConditionExpr()->Accept(this);
		} else {
			// If the condition is empty (e.g., for(;;)), it's an infinite loop. 
			// Materialize a constant true boolean (i1).
			builder_->retValue = builder_->op->create<mlir::arith::ConstantIntOp>(location, 1, 1);
		}
		
		mlir::Value condition = builder_->retValue;
		if (!condition)
			return;


		if (node->IteratorExpr()) {
			node->IteratorExpr()->Accept(this);
		}

		mlir::typher::YieldOp::create(*builder_->op, location, condition);

		mlir::Block* bodyBlock = builder_->op->createBlock(&whileOp.getBodyRegion());
		builder_->op->setInsertionPointToStart(bodyBlock);
		
		// Generate code for the actual loop statements
		builder_->GenBody(node->GetBody(), location);

		builder_->op->setInsertionPointAfter(whileOp);
		return;
	}

	void Generator::Visit(AST::WhileStatement* node) 
	{
		auto location = builder_->loc(node->Loc()); 

		auto whileOp = mlir::typher::WhileOp::create(*builder_->op, location);
		mlir::Block* condBlock = builder_->op->createBlock(&whileOp.getCondRegion());
		
		builder_->op->setInsertionPointToStart(condBlock);
		
		node->ConditionExpr()->Accept(this);
		mlir::Value condition = builder_->retValue;
		if (!condition)
			return ;

		//builder_->op->create<mlir::typher::ConditionYieldOp>(location, condVal);
		mlir::typher::YieldOp::create(*builder_->op, location, condition);

		mlir::Block* bodyBlock = builder_->op->createBlock(&whileOp.getBodyRegion());
		builder_->op->setInsertionPointToStart(bodyBlock);
		
		builder_->GenBody(node->GetBody(), location);

		builder_->op->setInsertionPointAfter(whileOp);
		return ; 
	}

	void Generator::Visit(AST::IfStatement* node) 
	{
		auto location = builder_->loc(node->Loc()); 
		node->ConditionExpr()->Accept(this);
		mlir::Value condition = builder_->retValue;
		if (!condition)
			return ;

		auto ifOp = mlir::typher::IfOp::create(*builder_->op, location, condition, 
			node->HasElse() || node->HasElif());

 		builder_->op->setInsertionPointToStart(&ifOp.getThenRegion().front());
		builder_->GenBody(node->GetBody(), location);
		
		if(node->HasElif() ) {
			builder_->op->setInsertionPointToStart(&ifOp.getElseRegion().front());
			llvm::ScopedHashTableScope<llvm::StringRef, mlir::Value> varScope(builder_->symbolTable);
			node->Elif()->Accept(this);
			builder_->op->setInsertionPointToEnd(&ifOp.getElseRegion().front());
			mlir::typher::YieldOp::create(*builder_->op, location);
		} else if (node->HasElse()) {
			builder_->op->setInsertionPointToStart(&ifOp.getElseRegion().front());
			builder_->GenBody(node->ElseBody(), location);
		}

		builder_->op->setInsertionPointAfter(ifOp);
		return ; 
	}

	void Generator::Visit(AST::Operator* node) 
	{
		auto location = builder_->loc(node->Loc());

		node->GetRHS()->Accept(this);
		mlir::Value rhs = builder_->LvalueToRvalue(builder_->retValue, location);

		if (!rhs)
			return;

		if(node->OperatorType() ==  AST::OperatorKind::ASN)
		{
			node->GetLHS()->Accept(this);
			mlir::Value lvalue = builder_->retValue;

			mlir::typher::AssignOp::create(*builder_->op, location, rhs, lvalue);

			return;
		}

		node->GetLHS()->Accept(this);
		mlir::Value lhs = builder_->LvalueToRvalue(builder_->retValue, location);;
		if (!lhs)
			return;

		switch(node->OperatorType()) {
			case AST::OperatorKind::ADD: {
				builder_->retValue = mlir::arith::AddIOp::create(*builder_->op, location, lhs, rhs);
				return;
			}
			case AST::OperatorKind::SUB: {
				builder_->retValue = mlir::arith::SubIOp::create(*builder_->op, location, lhs, rhs);
				return;
			}
			case AST::OperatorKind::MUL: {
				builder_->retValue = mlir::arith::MulIOp::create(*builder_->op, location, lhs, rhs);
				return;
			}
			case AST::OperatorKind::DIV: {
				builder_->retValue = mlir::arith::DivSIOp::create(*builder_->op, location, lhs, rhs);
				return;
			}
			case AST::OperatorKind::MOD: {
				builder_->retValue = mlir::arith::RemSIOp::create(*builder_->op, location, lhs, rhs);
				return;
			}
			default:{
				if (node->GetRHS()->Type()) { // TODO: should be decided in checker semantic analysis. The checker should decide
					     	   // which type the operation will yield via Type Promotion that uses "Path of Least Loss.".
					mlir::arith::CmpFPredicate predicate;
					switch (node->OperatorType()) {
						case AST::OperatorKind::EQS: predicate = mlir::arith::CmpFPredicate::OEQ; break;
						case AST::OperatorKind::NEQ: predicate = mlir::arith::CmpFPredicate::ONE; break;
						case AST::OperatorKind::LES: predicate = mlir::arith::CmpFPredicate::OLT; break;
						case AST::OperatorKind::LEQ: predicate = mlir::arith::CmpFPredicate::OLE; break;
						case AST::OperatorKind::GEQ: predicate = mlir::arith::CmpFPredicate::OGE; break;
						case AST::OperatorKind::GRT: predicate = mlir::arith::CmpFPredicate::OGT; break;
						default: /* handle error */ break;
					}
					builder_->retValue = mlir::arith::CmpFOp::create(*builder_->op, location, predicate, lhs, rhs);
				} else {
					mlir::arith::CmpIPredicate predicate;
					switch (node->OperatorType()) {
						case AST::OperatorKind::EQS: predicate = mlir::arith::CmpIPredicate::eq;  break;
						case AST::OperatorKind::NEQ: predicate = mlir::arith::CmpIPredicate::ne;  break;
						case AST::OperatorKind::LES: predicate = mlir::arith::CmpIPredicate::slt; break;
						case AST::OperatorKind::LEQ: predicate = mlir::arith::CmpIPredicate::sle; break;
						case AST::OperatorKind::GEQ: predicate = mlir::arith::CmpIPredicate::sge; break;
						case AST::OperatorKind::GRT: predicate = mlir::arith::CmpIPredicate::sgt; break;
						default: /* handle error */ break;
					}
					builder_->retValue = mlir::arith::CmpIOp::create(*builder_->op, location, predicate, lhs, rhs);
				}
				break;
			}
		}

		return;
	}
}
