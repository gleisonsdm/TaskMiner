//LLVM IMPORTS
#include "llvm/Support/CommandLine.h"

//LOCAL IMPORTS
#include "DepAnalysis.h"
#include "ControlDependenceGraph.h"

#define DEBUG_TYPE "dep-analysis"

using namespace llvm;

char DepAnalysis::ID = 0;
static RegisterPass<DepAnalysis> X("depanalysis", "Run the DepAnalysis algorithm. Generates a dependence graph", false, false);

static cl::opt<bool, false> printToDot("printToDot",
  cl::desc("Print dot file containing the depgraph"), cl::NotHidden);

STATISTIC(RARDeps, "RAR Total num of input dependences");
STATISTIC(WARDeps, "WAR Total num of anti dependences");
STATISTIC(RAWDeps, "RAW Total num of flow/true dependences");
STATISTIC(WAWDeps, "RAR Total num of output dependences");
STATISTIC(SCADeps, "SCA Total num of scalar dependences");
STATISTIC(CTRDeps, "CTR Total num of control dependences");

void DepAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {
	AU.setPreservesAll();
	AU.addRequired<PostDominatorTree>();
	AU.addRequired<DependenceAnalysis>();
	AU.addRequired<DominanceFrontier>();
	AU.addRequired<LoopInfoWrapperPass>();
	AU.addRequired<RegionInfoPass>();
}

bool DepAnalysis::runOnFunction(Function &F) {

	LoopInfoWrapperPass *LIWP = &(getAnalysis<LoopInfoWrapperPass>());
	LI = &(LIWP->getLoopInfo());

	RegionInfoPass *RIP = &(getAnalysis<RegionInfoPass>());
	RI = &(RIP->getRegionInfo());

	//Step1: Create PDG
	createProgramDependenceGraph(F);

	//Step2: Create RegionTree
	createRegionTree(F);

	// //Printing region tree
	// RT->print(errs());

	if (printToDot)
	{
		G->dumpToDot();
		RT->dumpToDot(F.getName(), true);
	}

	return true;
}

void DepAnalysis::createProgramDependenceGraph(Function &F)
{
	auto &DI = getAnalysis<DependenceAnalysis>();
	auto &PDT = getAnalysis<PostDominatorTree>();

	// This is going to represent the graph
	G = new PDG(F.getName(), &F);

	// Add all nodes to the graph
	for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
		for (BasicBlock::iterator i = BB->begin(), e = BB->end(); i != e; ++i)
			G->addNode(&*i);

	// Collect memory dependence edges
	for (inst_iterator SrcI = inst_begin(F), SrcE = inst_end(F); SrcI != SrcE; ++SrcI) {

		if (isa<StoreInst>(*SrcI) || isa<LoadInst>(*SrcI)) {

			for (inst_iterator DstI = SrcI, DstE = inst_end(F); DstI != DstE; ++DstI) {
				
				if (isa<StoreInst>(*DstI) || isa<LoadInst>(*DstI)) {

					DEBUG_WITH_TYPE("dep-analysis", errs() << "\nChecking [" << *SrcI << "] and [" << *DstI << "]\n");

					if (auto D = DI.depends(&*SrcI, &*DstI, true)) {
						if (D->isInput())
						{
							G->addEdge(&*SrcI, &*DstI, EdgeDepType::RAR);
							RARDeps++;
						}
						else if (D->isOutput())
						{
							G->addEdge(&*SrcI, &*DstI, EdgeDepType::WAW);
							WAWDeps++;
						}
						else if (D->isFlow())
						{
							G->addEdge(&*SrcI, &*DstI, EdgeDepType::RAW);
							RAWDeps++;
						}
						else if (D->isAnti())
						{
							G->addEdge(&*DstI, &*SrcI, EdgeDepType::RAWLC);
							WARDeps++;
						}
						else
							DEBUG_WITH_TYPE("dep-analysis", errs() << "Error decoding dependence type.\n");

						DEBUG_WITH_TYPE("dep-analysis", errs() << "\tDependent.\n");
					}
					else
						DEBUG_WITH_TYPE("dep-analysis", errs() << "\tIndependent.\n");
				}
			}
		}
	}

	// Collect data dependence edges
	for (inst_iterator SrcI = inst_begin(F), SrcE = inst_end(F); SrcI != SrcE; ++SrcI)
	{
		for (User *U : SrcI->users())
		{
			if (Instruction *Inst = dyn_cast<Instruction>(U))
			{
				G->addEdge(&*SrcI, Inst, EdgeDepType::SCA);
				SCADeps++;
			}
		}
	}

	// Collect control dependence edges
	ControlDependenceGraphBase cdgBuilder;
	cdgBuilder.graphForFunction(F, PDT);

	for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
	{
		BasicBlock *A = &*BB;
		auto term = dyn_cast<Instruction>(A->getTerminator());
		for (Function::iterator BB2 = F.begin(), E2 = F.end(); BB2 != E2; ++BB2)
		{
			BasicBlock *B = &*BB2;
			if (cdgBuilder.controls(A, B))
			{
				for (BasicBlock::iterator i = B->begin(), e = B->end(); i != e; ++i)
				{
					G->addEdge(term, &*i, EdgeDepType::CTR);
					CTRDeps++;
				}
			}
		}

		if (!cdgBuilder.isDominated(A)) {
			for (BasicBlock::iterator i = A->begin(), e = A->end(); i != e; ++i) {
				G->connectToEntry(&*i);
			}
		}

		if (cdgBuilder.getNode(A)->getNumChildren() == 0) {
			for (BasicBlock::iterator i = A->begin(), e = A->end(); i != e; ++i) {
				G->connectToExit(&*i);
			}
		}
	}
}

void DepAnalysis::createRegionTree(Function &F)
{
	// errs() << "\nCreating region tree for funntion F: " << F.getName() << "\n";
	RT = new RegionTree();
	std::set<Region*> allRegions;

	//Create all region wrappers
	std::map<Region*, RegionWrapper*> regionWrappers;
	// errs() << "\nIterating on basic blocks of this function...\n";
	for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
	{
		// errs() << BB->getName() << "\n";
		RegionWrapper* RW = new RegionWrapper();
		Region* R = RI->getRegionFor(BB);
		if (R == nullptr)
			continue;
		regionWrappers[R] = RW;

		// R->dump();

		RW->topLevel = R->isTopLevelRegion();
		Loop* L = LI->getLoopFor(BB);
		RW->F = &F;
		if (L != nullptr)
		{
			RW->hasLoop = true;
		}
	}

	// errs() << "\nInsterting nodes in region tree\n";
	//Collect all region data and insert the nodes in the region tree
	for (auto &r : regionWrappers)
	{
		r.second->entry = r.first->getEntry();
		r.second->exit = r.first->getExit();
		if (!r.first->isTopLevelRegion())
			r.second->parent = regionWrappers[r.first->getParent()];
		else
			r.second->parent = nullptr;
		r.second->F = &F;

		RT->addNode(r.second);
	}
	
	//Collect dependencies edges
	Edge<RegionWrapper*, EdgeDepType> *e_;
	for (auto e : G->getEdges())
	{
		if ((e->getType() != EdgeDepType::WAW)
			&& (e->getType() != EdgeDepType::RAR)
			&& (e->getType() != EdgeDepType::SCA) 
			&& (e->getSrc() != G->getEntry()) 
			&& (e->getDst() != G->getExit()))
		{
			Region *src = RI->getRegionFor(e->getSrc()->getItem()->getParent());
			Region *dst = RI->getRegionFor(e->getDst()->getItem()->getParent());
			if (src != dst)
			{
				if (!(e_ = RT->checkEdge(regionWrappers[src], regionWrappers[dst], e->getType())))
				{
					e_ = RT->addEdge(regionWrappers[src], regionWrappers[dst], e->getType());
				}
				e_->addIntensity();
			}			
		}
	}

	//Collect Recursive Edges
	for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
		for (BasicBlock::iterator i = BB->begin(), e = BB->end(); i != e; ++i)
		{
			if (CallInst* CI = dyn_cast<CallInst>(i))
			{
				if ((CI->getCalledFunction()) == &F)
				{
					Region *src = RI->getRegionFor(BB);
					Region *dst = RI->getTopLevelRegion();
					RT->addEdge(regionWrappers[src], regionWrappers[dst], EdgeDepType::RECURSIVE);
				}
			}
		}

	//Collect parent edges
	for (auto n : RT->getNodes())
	{
		if (n->getItem()->parent != nullptr)
		{
			RT->addEdge(n->getItem()->parent, n->getItem(), EdgeDepType::PARENT);
		}
	}
}


