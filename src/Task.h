#ifndef TASK_H
#define TASK_H

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/RegionInfo.h"

//Local imports
//include "CostModel.h"

#include <set>
#include <stack>
#include <list>

namespace llvm
{
	class Task;
	class FunctionCallTask;
	class RegionTask;
	class RecursiveTask;

	enum AccessType {UNKNOWN=0, READ=1, WRITE=2, READWRITE=3};

	inline AccessType operator|(AccessType a, AccessType b)
	{
		return static_cast<AccessType>(static_cast<int>(a) | static_cast<int>(b)); 
	}

	class Task
	{
	public:
		enum TaskKind
		{
			FCALL_TASK,
			REGION_TASK,
			RECURSIVE_TASK
		};

		Task(TaskKind k) 
			: kind(k)
			{}
		virtual ~Task() {};
		
		//Getters & Setters
		TaskKind getKind() const { return kind; }
		std::set<Value*> getLiveIN() const;
		std::set<Value*> getLiveOUT() const;
		std::set<Value*> getLiveINOUT() const;
		std::set<BasicBlock*> getbbs() const;
		//CostModel getCost() const { return CM; }
		std::set<Value*> getPrivateValues() { return privateValues; };
		void addPrivateValue(Value* V) { privateValues.insert(V); };
		std::set<Value*> getSharedValues() { return sharedValues; };
		void addSharedValue(Value* V) { sharedValues.insert(V); };
		Function* getParentFunction();

		//Methods
		virtual bool resolveInsAndOutsSets() { return false; }
		virtual void resolvePrivateValues();
	  //	virtual CostModel computeCost() { return CM; }
		void addBasicBlock(BasicBlock* bb) { bbs.insert(bb); }
		bool hasLoadInstructionInDependence() const;
		virtual bool hasSyncBarrier() const {return false; }
		bool isSafeForAnnotation() const;
		bool isGlobal(Value* I) const;

		//Printing to output stream methods
		virtual raw_ostream& print(raw_ostream& os) const;
		raw_ostream& printLiveSets(raw_ostream& os) const;
		raw_ostream& printPrivateValues(raw_ostream& os) const;

	private:
		const TaskKind kind;

	protected:
		//Cost model
		//CostModel CM;

		//Content:
		std::set<BasicBlock*> bbs;
		std::set<Value*> liveIN;
		std::set<Value*> liveOUT;
		std::set<Value*> liveINOUT;
		Loop* outerMost=0;
		std::set<Value*> privateValues;
		std::set<Value*> sharedValues;

		//Private methods
		AccessType getTypeFromInst(Instruction* I);
		std::string accessTypeToStr(AccessType T);
		bool isPointerValue(Value *V);

	};

	class FunctionCallTask : public Task
	{
	public:
		FunctionCallTask(CallInst* CI);
		~FunctionCallTask() {}
		CallInst* getFunctionCall() const;
		bool resolveInsAndOutsSets() override;
		//CostModel computeCost() override;
		raw_ostream& print(raw_ostream& os) const override;
		static bool classof(const Task* T) { return T->getKind() == FCALL_TASK; }
		bool hasSyncBarrier() const override;
		void resolvePrivateValues() override;

	private:
		CallInst* functionCall;
		bool syncBarrier=true;
	};

	class RegionTask : public Task
	{
	public:
		RegionTask() 
			: Task(REGION_TASK)
			{ level = 0; }
		~RegionTask() {}
		bool resolveInsAndOutsSets() override;
		//CostModel computeCost() override;
		raw_ostream& print(raw_ostream& os) const override;
		void resolvePrivateValues() override;
		static bool classof(const Task* T) { return T->getKind() == REGION_TASK; }
		void setHeaderBB(BasicBlock* BB) { header = BB; };
		BasicBlock* getHeaderBB() { return header; };

	private:
		int level;
		BasicBlock* header;
	};

	class RecursiveTask : public Task
	{
	public:
		RecursiveTask(CallInst *CI, bool isInsideLoop);
		~RecursiveTask() {}
		CallInst* getRecursiveCall() const;
		bool resolveInsAndOutsSets() override;
		//CostModel computeCost() override;
		raw_ostream& print(raw_ostream& os) const override;
		void resolvePrivateValues() override;
		static bool classof(const Task* T) { return T->getKind() == RECURSIVE_TASK; }		
		RecursiveTask* getPrev() const { return prev; }
		RecursiveTask* getNext() const { return next; }
		void setPrev(RecursiveTask* prev) { this->prev = prev; }
		void setNext(RecursiveTask* next) { this->next = next; }
		bool hasSyncBarrier() const override;
		bool insideLoop() { return isInsideLoop; }
		uint32_t getBaseCaseCost() const;

	private:
		RecursiveTask* prev=nullptr;
		RecursiveTask* next=nullptr;
		CallInst *recursiveCall;
		bool isInsideLoop=false;
	};
}

#endif
