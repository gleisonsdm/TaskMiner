#include "llvm/Pass.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/CallGraph.h"

//LOCAL IMPORTS
#include "DepAnalysis.h"
#include "RegionTree.h"
#include "Graph.hpp"
#include "Task.h"
#include "CostModel.h"
#include "TaskMinerUtils.h"

namespace llvm
{
	class TaskMiner : public ModulePass
	{
	private:
		//ModuleTaskGraph 
		RegionTree* taskGraph;
		// DepAnalysis *DP=0;
		std::map<Function*, RegionTree*> RTs;	
		std::map<Function*, RegionTree*> getAllRegionTrees(Module &M);

		//tasks collected for this module
		std::list<Task*> tasks;

		//record of each instcall and their edges in the task graph
		std::map<Edge<RegionWrapper*, EdgeDepType>*, CallInst*> callInsts;

		//record of every scc in the taskgraph
		std::set<Graph<RegionWrapper*, EdgeDepType>* > SCCs;

		//keep alias sets
		// std::map<Value*, AliasSet> aliassets;

		//Keep functions that have been turned into tasks
		std::set<Function*> function_tasks;

		//List of top level recursive function calls
		std::set<CallInst*> topLevelRecCalls;

		//Utils module
		TaskMinerUtils TMU;

	public:
		static char ID;
		static uint32_t N_WORKERS; //NUMBER OF THREADS
		static uint32_t RUNTIME_COST; //RUNTIME COST
		static const uint32_t THRESHOLD = 1; //THRESHOLD	

		TaskMiner() : ModulePass(ID) {}
		~TaskMiner() { /*delete taskGraph;*/ }
		void getAnalysisUsage(AnalysisUsage &AU) const override;
		bool runOnModule(Module &M) override;
		RegionTree* gettaskGraph(Module &M);
		bool findRegionWrapperInSCC(RegionWrapper* RW);
		std::list<CallInst*> getLastRecursiveCalls() const;
		void mineRegionTasks(Module &M);
		void mineRecursiveTasks();
		void mineFunctionCallTasks();
		void mineLoopTasks();
		void mineTasks(Module &M);
		void resolveInsAndOutsSets();
		void resolvePrivateValues();
		void computeCosts();
		void computeStats(Module &M);
		void computeTotalCost();
		void determineTopLevelRecursiveCalls(Module &M);
		bool isRecursive(Function &F, CallGraph &CG);
		void updateCoveredFunctions(std::set<Function*> &set);
		void findTopLevelFunctionCall(Function &callee, Function &caller);
		std::list<Task*> getTasks() { return tasks; }
		std::set<CallInst*> getTopLevelRecCalls() { return topLevelRecCalls; };

		//printing methods
		void printTasks();
		void printRegionInfo();



	};
}