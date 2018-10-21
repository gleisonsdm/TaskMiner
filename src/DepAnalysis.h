#ifndef DEP_ANALYSIS_H
#define DEP_ANALYSIS_H

//LLVM IMPORTS
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/RegionInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/DominanceFrontier.h"
#include "llvm/Analysis/PostDominators.h"

//STL IMPORTS
#include <set>
#include <list>
#include <map>
#include <string>
#include <fstream>
#include <iomanip>

//LOCAL IMPORTS
#include "ControlDependenceGraph.h"
#include "PDG.h"
#include "RegionTree.h"

namespace llvm {

	class DepAnalysis : public FunctionPass
	{
	private:
		PDG* G=0;
		RegionTree *RT=0;
		void createProgramDependenceGraph(Function &F);
		void createRegionTree(Function &F);
		LoopInfo *LI=0;
		RegionInfo *RI=0;

	public:	
		static char ID;
		DepAnalysis() : FunctionPass(ID) {}
		~DepAnalysis() {}
		void releaseMemory() override {}
		void getAnalysisUsage(AnalysisUsage &AU) const override;
		bool runOnFunction(Function &F) override;
		PDG* getDepGraph() { return G; }
		RegionTree* getRegionTree() { return RT; }
	};

}



#endif

