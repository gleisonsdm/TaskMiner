#ifndef REGION_TREE_H
#define REGION_TREE_H

//LLVM IMPORTS
#include "llvm/IR/Instructions.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/RegionInfo.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/InstIterator.h"

//STL IMPORTS
#include <string>

//LOCAL IMPORTS
#include "Graph.hpp"
#include "EdgeDepType.h"

using namespace llvm;

//Forward declarations
struct RegionWrapper;

class RegionTree : public Graph<RegionWrapper*, EdgeDepType>
{
private:
	std::set<Graph<RegionWrapper*, EdgeDepType>* > scSubgraphs;
	std::map<Edge<RegionWrapper*, EdgeDepType>*, CallInst*> fCallInsts;

public:
	RegionTree() {};
	~RegionTree() {};
	std::set<Graph<RegionWrapper*, EdgeDepType>* > getStronglyConnectedSubgraphs();
	void dumpToDot(std::string filename, bool dep=false);
	std::string edgeLabel(Edge<RegionWrapper*, EdgeDepType> *e) const;
	raw_ostream& print(raw_ostream& os=errs()) const;

	Node<RegionWrapper*>* getNode(BasicBlock* entry, BasicBlock* exit, bool topLevel, bool isHub=false);
	Node<RegionWrapper*>* getNode(RegionWrapper* rw);
	Node<RegionWrapper*>* getTopLevelRegionNode(Function *F, bool isHub=false);
	void addHubFunction(Node<RegionWrapper*>* src, RegionTree* cpy);
	bool compareTwoRegionWrappers(RegionWrapper* original, RegionWrapper* hub);
	RegionTree* copyRegionTree(RegionTree* src);
	RegionWrapper* copyRegionWrapper(RegionWrapper* src);



};

struct RegionWrapper
{
	BasicBlock* entry=0;
	BasicBlock* exit=0;
	Function* F=0;
	RegionWrapper* parent=0;
	Loop* L=0;

	bool isHub=false;
	bool topLevel=false;
	bool hasLoop=false;

	BasicBlock* getEntry() { return entry; }
};

#endif // REGION_TREE_H