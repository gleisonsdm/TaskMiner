#ifndef PDG_H
#define PDG_H

//LLVM IMPORTS
#include "llvm/IR/Instructions.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/DominanceFrontier.h"
#include "llvm/Analysis/PostDominators.h"

//STL IMPORTS
#include <string>

//LOCAL IMPORTS
#include "Graph.hpp"
#include "EdgeDepType.h"

#define ENTRY 1000000
#define EXIT 2000000

using namespace llvm;

class PDG : public Graph<Instruction*, EdgeDepType>
{
private:
	std::string functionName;
	Function *F;
	Node<Instruction*> *entry;
	Node<Instruction*> *exit;
	std::set<Graph<Instruction*, EdgeDepType>* > scSubgraphs;

public:
	PDG(std::string fName, Function *F)
		: functionName(fName)
		, F(F)
		, entry(nullptr)
		, exit(nullptr)
		{
			entry = addNode((Instruction*)ENTRY);
			exit = addNode((Instruction*)EXIT);
		}
	PDG()
		: functionName("NONE")
		, F(nullptr)
		, entry(nullptr)
		, exit(nullptr)
		{}
	~PDG()
	{
		delete entry;
		delete exit;
	}

	void dumpToDot();
	std::string edgeLabel(Edge<Instruction*, EdgeDepType> *e);
	void connectToEntry(Instruction* inst);
	void connectToExit(Instruction* inst);
	Node<Instruction*> *getEntry() { return entry; }
	Node<Instruction*> *getExit() { return exit; }
	std::set<Graph<Instruction*, EdgeDepType>* > getStronglyConnectedSubgraphs();
};

#endif // PDG_H
