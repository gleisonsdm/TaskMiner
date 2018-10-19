#ifndef _COST_MODEL_H_
#define _COST_MODEL_H_

//LLVM IMPORTS
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/RegionInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

class CostModel
{
private:
	uint32_t nInsts; // TASKSIZE
	static uint32_t nWorkers; //NUMBER OF THREADS
	static uint32_t runtimeCost; //RUNTIME COST
	static const uint32_t THRESHOLD = 1; //THRESHOLD	
	static uint32_t singleTaskCost;

	uint32_t tripcount=10; //LOOP'S TRIP COUNT
	uint32_t nInDeps; // NUMBER OF IN-DEPS
	uint32_t nOutDeps; // NUMBER OF OUT-DEPS

public:
	CostModel() {};
	CostModel(uint32_t totalInsts, uint32_t workers)
	{
		this->singleTaskCost = totalInsts;
		this->nWorkers = workers;
	}
	~CostModel() {};

	static void setRuntimeCost(uint32_t rCost);
	static void setNWorkers(uint32_t nWorkers);
	
	void setTripCount(uint32_t tripcount)
	{
		this->tripcount = tripcount;
	}

	void setData(uint32_t nInsts, uint32_t nInDeps, uint32_t nOutDeps)
	{
		this->nInsts = nInsts;
		this->nInDeps = nInDeps;
		this->nOutDeps = nOutDeps;
	}

	uint32_t getCost() const { return (getIdealCost()/runtimeCost); }
	bool aboveThreshold() { return getCost() >= THRESHOLD; }
	uint32_t getNInsts() { return nInsts; }
	uint32_t getNInDeps() { return nInDeps; }
	uint32_t getNOutDeps() { return nOutDeps; }
	uint32_t getIdealCost() const { return singleTaskCost/nWorkers; }

	llvm::raw_ostream& print(llvm::raw_ostream& os) const
	{
		os << "\nTASK COST:\n";
		os << nInsts << " instructions\n";
		os << nInsts*tripcount << " instructions * tripcount\n";
		os << nWorkers << " threads\n";
		os << nInDeps << " input dependencies\n";
		os << nOutDeps << " output dependencies\n";
		os << getCost() << " as the computed cost (ideal_cost/runtime_cost)\n\n";
	
		return os;
	}

};


#endif // _COST_MODEL_H_