#include "llvm/Analysis/LoopInfo.h" 
#include "llvm/IR/Instructions.h"
#include "Task.h"

#include <stack>

using namespace llvm;

std::set<Value*> Task::getLiveIN() const { return liveIN; }
std::set<Value*> Task::getLiveOUT() const { return liveOUT; }
std::set<Value*> Task::getLiveINOUT() const { return liveINOUT; }
std::set<BasicBlock*> Task::getbbs() const { return bbs; }

bool Task::isSafeForAnnotation() const
{
	for (auto bb : bbs)
	{
		for (BasicBlock::iterator inst = bb->begin(); inst != bb->end(); inst++)
		{
			if (GetElementPtrInst* GEPI = dyn_cast<GetElementPtrInst>(inst))
			{
				if (isGlobal(GEPI))
					return false;
			}
		}
	}

	return true;
}

bool Task::isGlobal(Value* I) const
{
	if (GlobalValue* GV = dyn_cast<GlobalValue>(I))
		return true;
	else if (LoadInst* LI = dyn_cast<LoadInst>(I))
		return isGlobal(LI->getPointerOperand());
	else if (StoreInst* SI = dyn_cast<StoreInst>(I))
		return isGlobal(SI->getPointerOperand());
	else if (GetElementPtrInst* GEPI = dyn_cast<GetElementPtrInst>(I))
		return isGlobal(GEPI->getPointerOperand());
/*	else if (PHINode* phi = dyn_cast<PHINode>(I))
	{
		bool phiOperands = true;
		for (int i = 0; i < phi->getNumOperands(); i++)
			phiOperands |= isGlobal(phi->getOperand(i));

		return phiOperands;
	}*/

	return false;
}

AccessType Task::getTypeFromInst(Instruction* I)
{
	if (dyn_cast<GetElementPtrInst>(I))
	{
		AccessType T = AccessType::UNKNOWN;
		for (auto user : I->users())
		{
			if (Instruction* inst = dyn_cast<Instruction>(user))
				T = T | getTypeFromInst(inst);
		}
		return T;
	}
	else if (dyn_cast<LoadInst>(I)) return AccessType::READ;
	else if (dyn_cast<StoreInst>(I)) return AccessType::WRITE;
	else return AccessType::UNKNOWN;
}

std::string Task::accessTypeToStr(AccessType T)
{
	switch(T)
	{
		case AccessType::READ:
			return "READ";
		case AccessType::WRITE:
		 	return "WRITE";
		case AccessType::READWRITE:
			return "READWRITE";
		case AccessType::UNKNOWN:
			return "UNKNOWN";
		default:
			llvm_unreachable("T should be one of the 4 access types: 0-UNK, 1-REA, 2-WRT, 3-R/W");
	}
}

raw_ostream& Task::printPrivateValues(raw_ostream &os) const
{
	os << "\nPRIVATE VALUES:\n";
	for (auto v : privateValues)
	{
		os << "\t";
		v->print(os);
		os << "\n";
	}
	os << "\nSHARED VALUES:\n";
	for (auto v : sharedValues)
	{
		os << "\t";
		v->print(os);
		os << "\n";
	}
}

raw_ostream& Task::printLiveSets(raw_ostream& os) const
{
	os << "\nIN:\n";
	for (auto &in : liveIN)
	{
		os << "\t";
		in->print(os);
		os << "\n";
	}
	os << "OUT:\n\t";
	for (auto &out : liveOUT)
	{
		os << "\t";
		out->print(os);
		os << "\n";
	}
	os << "INOUT:\n\t";
	for (auto &inout : liveINOUT)
	{
		os << "\t";
		inout->print(os);
		os << "\n";
	}

	return os;
}

raw_ostream& Task::print(raw_ostream& os) const
{
	if (hasLoadInstructionInDependence())
		os << "\nTASK HAS LOAD INSTRUCTION IN DEPENDENCIES\n";
	printPrivateValues(os);
	return printLiveSets(os);
}

bool Task::hasLoadInstructionInDependence() const
{
	for (auto I : getLiveINOUT())
		if (isa<LoadInst>(I) || isa<StoreInst>(I))
			return true;
	for (auto I : getLiveOUT())
		if (isa<LoadInst>(I) || isa<StoreInst>(I))
			return true;
	for (auto I : getLiveIN())
		if (isa<LoadInst>(I) || isa<StoreInst>(I))
			return true;

	return false;
}

Function* Task::getParentFunction()
{
	if (bbs.empty())
		return nullptr;

	return (*(bbs.begin()))->getParent();
}

void Task::resolvePrivateValues()
{
	Function* F = getParentFunction();
	for (Function::iterator ft = F->begin(); ft != F->end(); ft++)
	{
		for (BasicBlock::iterator bt = ft->begin(); bt != ft->end(); bt++)
		{
			Instruction* i = bt;
		  BasicBlock* p = i->getParent();		  
		  if (bbs.find(p) != bbs.end())
		  	continue;
		  
   		for (auto u : i->users())
  		{
  			Instruction* inst = dyn_cast<Instruction>(u);
   	 		if (!inst)
   	 			continue;

				BasicBlock* pu = inst->getParent();
				bool isInsideTask = (bbs.find(pu) != bbs.end());
      	if (!isInsideTask)
      		continue;

   			privateValues.insert(i);	
	    }
		}
	}
}

FunctionCallTask::FunctionCallTask(CallInst* CI)
	: Task(FCALL_TASK)
	, functionCall(CI)
	{
		Function* F = functionCall->getCalledFunction();
		for (Function::iterator BB = F->begin(); BB != F->end(); ++BB)
		{
			bbs.insert(BB);
		}
	}

raw_ostream& FunctionCallTask::print(raw_ostream& os) const
{
	os << "\n===========\n" 
					<< "Type: Function Call Task"
					<< "\n===========\n"
					<< "Function Call: \n\t";
	functionCall->print(os);
	printLiveSets(os);
	printPrivateValues(os);
	//CM.print(os);
	os << "\nHas sync barrier?\n";
	os << this->hasSyncBarrier();
	// this->outerMost->print(os);
	os << "\n";
	os << "Is safe for annotation? ";
	if (isSafeForAnnotation())
		os << " yes\n";
	else
		os << " no\n";

	return os;
}

bool FunctionCallTask::resolveInsAndOutsSets()
{
	Function* F = functionCall->getCalledFunction();
	std::map<Value*, AccessType> parameterAccessType;
	std::map<Value*, Value*> matchArgsParameters;
	Value* V;

	//Match parameters with arguments
	auto arg_aux = F->arg_begin();
	for (unsigned i = 0; i < functionCall->getNumArgOperands(); i++)
	{
		V = functionCall->getArgOperand(i);
		matchArgsParameters[arg_aux] = V;
		arg_aux++;
	}

	//Find the access types of the parameters
	// errs() << "Function " << F->getName() << ":\n";
	for (auto &arg : F->getArgumentList())
	{
		// arg.print(errs());
		// errs() << "\n";
		if (!arg.user_empty())
			for (auto user : arg.users())
			{
				if (Instruction* I = dyn_cast<Instruction>(user))
				{
					AccessType T = getTypeFromInst(I);
					if (parameterAccessType.find(&arg) == parameterAccessType.end())
						parameterAccessType[&arg] = T;
					else
						parameterAccessType[&arg] = parameterAccessType[&arg] | T;
				}
			}
	}

	//Resolve ins and outs sets
	for (auto &p : parameterAccessType)
	{
		if (!isPointerValue(matchArgsParameters[p.first]))
			continue;
		switch (p.second)
		{
			case AccessType::READ:
				liveIN.insert(matchArgsParameters[p.first]);
				break;
			case AccessType::WRITE:
				liveOUT.insert(matchArgsParameters[p.first]);
				break;
			case AccessType::READWRITE:
				liveINOUT.insert(matchArgsParameters[p.first]);
				break;
			case AccessType::UNKNOWN:
				liveIN.insert(matchArgsParameters[p.first]);
				break;
		}
	}

	return true;
}

CallInst* FunctionCallTask::getFunctionCall() const { return functionCall; }

/*CostModel FunctionCallTask::computeCost()
{
	uint32_t n_insts = 0;
	uint32_t n_indeps = liveIN.size();
	uint32_t n_outdeps = liveOUT.size();

	Function* F = functionCall->getCalledFunction();
	std::stack<Function*> funcs;
	funcs.push(F);
	while (!funcs.empty())
	{
		Function* func = funcs.top();
		funcs.pop();
		if (func->empty())
			continue;
		for (Function::iterator bb = func->begin(); bb != func->end(); ++bb)
			for (BasicBlock::iterator it = bb->begin(); it != bb->end(); ++it)
			{
				n_insts++;
				if (CallInst* CI = dyn_cast<CallInst>(it))
				{
					funcs.push(CI->getCalledFunction());
				}
			}
	}

	CM.setData(n_insts, n_indeps, n_outdeps);

	return CM;
}*/


bool FunctionCallTask::hasSyncBarrier() const
{
	return this->syncBarrier;
}

void FunctionCallTask::resolvePrivateValues()
{
	Task::resolvePrivateValues();
	for (auto &arg : functionCall->arg_operands())
	{
		if (isPointerValue(arg) && isa<AllocaInst>(arg)) {
 			sharedValues.insert(arg);
    }
	}
}


RecursiveTask::RecursiveTask(CallInst *CI, bool isInsideLoop)
	: recursiveCall(CI)
	, isInsideLoop(isInsideLoop)
	, Task(RECURSIVE_TASK)
	{
		Function* F = recursiveCall->getCalledFunction();
		for (Function::iterator BB = F->begin(); BB != F->end(); ++BB)
		{
			bbs.insert(BB);
		}		
	}

CallInst* RecursiveTask::getRecursiveCall() const
{
	return recursiveCall;
}

bool RecursiveTask::resolveInsAndOutsSets()
{
	Function* F = recursiveCall->getCalledFunction();
	std::map<Value*, AccessType> parameterAccessType;
	std::map<Value*, Value*> matchArgsParameters;
	Value* V;

	//Match parameters with arguments
	auto arg_aux = F->arg_begin();
	for (unsigned i = 0; i < recursiveCall->getNumArgOperands(); i++)
	{
		V = recursiveCall->getArgOperand(i);
		matchArgsParameters[arg_aux] = V;
		arg_aux++;
	}

	//Find the access types of the parameters
	// errs() << "Function " << F->getName() << ":\n";
	for (auto &arg : F->getArgumentList())
	{
		// arg.print(errs());
		// errs() << "\n";
		if (!arg.user_empty())
			for (auto user : arg.users())
			{
				if (Instruction* I = dyn_cast<Instruction>(user))
				{
					AccessType T = getTypeFromInst(I);
					if (parameterAccessType.find(&arg) == parameterAccessType.end())
						parameterAccessType[&arg] = T;
					else
						parameterAccessType[&arg] = parameterAccessType[&arg] | T;
				}
			}
	}

	//Resolve ins and outs sets
	for (auto &p : parameterAccessType)
	{
		if (!isPointerValue(matchArgsParameters[p.first]))
			continue;
		switch (p.second)
		{
			case AccessType::READ:
				liveIN.insert(matchArgsParameters[p.first]);
				break;
			case AccessType::WRITE:
				liveOUT.insert(matchArgsParameters[p.first]);
				break;
			case AccessType::READWRITE:
				liveINOUT.insert(matchArgsParameters[p.first]);
				break;
			case AccessType::UNKNOWN:
				liveIN.insert(matchArgsParameters[p.first]);
				break;
		}
	}

	return true;
}

raw_ostream& RecursiveTask::print(raw_ostream& os) const
{
	os << "\n===========\n" 
					<< "Type: Recursive Call Task"
					<< "\n===========\n"
					<< "Recursive Call: \n\t";
	recursiveCall->print(os);
	os << "\nHas sync barrier afterwards? ";
	os << ((hasSyncBarrier()) ? "Yes" : "No");
	os << "\n";
	printLiveSets(os);
	printPrivateValues(os);
	//CM.print(os);
	os << "Total Insts: " << getBaseCaseCost() << "\n";
	os << "Is safe for annotation? ";
	if (isSafeForAnnotation())
		os << " yes\n";
	else
		os << " no\n";

	return os;	
}

/*CostModel RecursiveTask::computeCost()
{
	uint32_t n_insts = 0;
	uint32_t n_indeps = liveIN.size();
	uint32_t n_outdeps = liveOUT.size();

	Function* func = recursiveCall->getCalledFunction();
	for (Function::iterator bb = func->begin(); bb != func->end(); ++bb)
			for (BasicBlock::iterator it = bb->begin(); it != bb->end(); ++it)
				n_insts++;

	CM.setData(n_insts, n_indeps, n_outdeps);

	return CM;
}*/

bool RecursiveTask::hasSyncBarrier() const
{ 
	return (next == nullptr) ? true : false;
}

void RecursiveTask::resolvePrivateValues()
{
	Task::resolvePrivateValues();
	for (auto &arg : recursiveCall->arg_operands())
	{
		if (isPointerValue(arg) && isa<AllocaInst>(arg))
			sharedValues.insert(arg);
	}
}

uint32_t RecursiveTask::getBaseCaseCost() const
{
	uint32_t totalInsts = 0;
	std::stack<Function*> functions;
	Function* F = recursiveCall->getCalledFunction();
	for (Function::iterator bb = F->begin(); bb != F->end(); ++bb)
	{
		for (BasicBlock::iterator inst = bb->begin(); inst != bb->end(); ++inst)
		{
			// errs() << "Counting inst: ";
			// inst->print(errs());
			totalInsts++;
			if (CallInst* CI = dyn_cast<CallInst>(inst))
			{
				if (CI->getCalledFunction() == F || CI->getCalledFunction()->empty())
					continue;
				// errs() << "Push into the stack: ";
				// CI->print(errs());
				// errs() << "\n";
				functions.push(CI->getCalledFunction());
			}
		}
	}

	while (!functions.empty()) 
	{
		auto F = functions.top();
		functions.pop();
		// errs() << "Pop the stack: ";
		// errs() << F->getName();
		// errs() << "\n";

		if (F->empty() || F->isIntrinsic())
			continue;
		for (auto bb = F->begin(); bb != F->end(); ++bb)
			for (auto inst = bb->begin(); inst != bb->end(); ++inst)
			{
				totalInsts++;
				if (CallInst* CI = dyn_cast<CallInst>(inst))
				{
					// errs() << "Push into the stack: ";
					// CI->print(errs());
					// errs() << "\n";
					if (CI->getCalledFunction() == F || CI->getCalledFunction()->empty())
						continue;
					functions.push(CI->getCalledFunction());
				}
			}
	}

	return totalInsts;
}

bool RegionTask::resolveInsAndOutsSets()
{
	//Collect values inside region
	std::map<Value*, AccessType> parameterAccessType;
	for (auto bb : getbbs())
	{
		for (BasicBlock::iterator I = bb->begin(); I != bb->end(); ++I)
		{
			if (!isPointerValue(I))
				continue;
			if (LoadInst* LI = dyn_cast<LoadInst>(I))
			{
				Value* v = LI->getPointerOperand();
				if (parameterAccessType.find(v) == parameterAccessType.end())
					parameterAccessType[v] = AccessType::UNKNOWN;
				parameterAccessType[v] = parameterAccessType[v] | AccessType::READ;
			}
			if (StoreInst* SI = dyn_cast<StoreInst>(I))
			{
				Value* v = SI->getPointerOperand();
				if (parameterAccessType.find(v) == parameterAccessType.end())
					parameterAccessType[v] = AccessType::UNKNOWN;
				parameterAccessType[v] = parameterAccessType[v] | AccessType::WRITE;
			}
		}
	}

	//Resolve ins and outs sets
	for (auto &p : parameterAccessType)
	{
		if (!isPointerValue(p.first))
			continue;
		switch (p.second)
		{
			case AccessType::READ:
				liveIN.insert(p.first);
				break;
			case AccessType::WRITE:
				liveOUT.insert(p.first);
				break;
			case AccessType::READWRITE:
				liveINOUT.insert(p.first);
				break;
			case AccessType::UNKNOWN:
				liveIN.insert(p.first);
				break;
		}
	}

	return true;
}

void RegionTask::resolvePrivateValues()
{
	Task::resolvePrivateValues();
  for (BasicBlock::iterator i = header->begin(); i != header->end(); ++i)
	{
		for (auto u : i->users())
		{
			Instruction* user = dyn_cast<Instruction>(u);
			if (!user)
				continue;
		 	
	  	BasicBlock* pu = user->getParent();
	  	bool isInsideTask = bbs.find(pu) != bbs.end() && pu != header;
	  	if (!isInsideTask)
	  		continue;

     	privateValues.insert(i);
		}
	}
}

bool Task::isPointerValue(Value *V)
{
	if (isa<Argument>(V) || isa<GlobalValue>(V))
	{
		return V->getType()->isPtrOrPtrVectorTy();
	}
	if (!isa<LoadInst>(V) &&
	  !isa<StoreInst>(V) &&
	  !isa<GetElementPtrInst>(V) &&
	  !isa<AllocaInst>(V))
	{
		return false;
	}

return true;
}

raw_ostream& RegionTask::print(raw_ostream& os) const
{
	os << "\n===========\n" 
					<< "Type: Region Task"
					<< "\n===========\n"
					<< "At: \n";
	os << this->header->getParent()->getName() << "\n"; 
	os << "BBs: \n\t";
	for (auto bb : bbs)
		os << " " << bb->getName();
	printLiveSets(os);
	printPrivateValues(os);
	//CM.print(os);
	os << "\nEntry BB:" << header->getName() << "\n";
	os << "Is safe for annotation? ";
	if (isSafeForAnnotation())
		os << " yes\n";
	else
		os << " no\n";

	return os;	
}

/*CostModel RegionTask::computeCost()
{
	uint32_t n_insts = 0;
	uint32_t n_indeps = liveIN.size();
	uint32_t n_outdeps = liveOUT.size();

	for (auto bb : bbs)
			for (BasicBlock::iterator it = bb->begin(); it != bb->end(); ++it)
				n_insts++;

	CM.setData(n_insts, n_indeps, n_outdeps);

	return CM;
}*/
