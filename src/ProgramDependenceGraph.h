#ifndef PROGRAM_DEPENDENCE_GRAPH_H
#define PROGRAM_DEPENDENCE_GRAPH_H

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

using namespace llvm;

#include <set>
#include <list>
#include <map>
#include <string>
#include <fstream>
#include <stack>

// Only edge type are necessary for now. We don't keep track of distances.
enum DependenceType {RAR, WAW, RAW, WAR, CTR, SCA, RAWLC};

// Currently GraphNode is just a wrap around Instruction*, however
// I imagine that in the future we will want to add other properties
// to PDG nodes which is what motivated the creation of this class.
class GraphNode {
public:
	Instruction* instr;

	GraphNode(Instruction* _instr) : instr(_instr) {}
};

// Represent an edge in the PDG. We store whether the edge is a data
// or control dependence as well as the source and sink of the dependence.
class GraphEdge
{
public:
	DependenceType type;
	GraphNode* src;
	GraphNode* dst;

	GraphEdge(GraphNode* _src, GraphNode* _dst, DependenceType _type)
		: type(_type), src(_src), dst(_dst) {}

	std::string edgeLabel();
};


class ProgramDependenceGraph {

	unsigned int nextInstrID;

	private:
		GraphNode* getGraphNode(Instruction* instr);

	public:
		// Name of the function that this graph was created from. Just for reference/debugging/printing.
		std::string functionName;

		GraphNode* entry;
		GraphNode* exit;

		// using this data structure we can easily obtain all edges leaving a node
		std::map<GraphNode*, std::set<GraphEdge*>> outEdges;

		// using this data structure we can easily obtain all edges directly reaching a node
		std::map<GraphNode*, std::set<GraphEdge*>> inEdges;

		// we use this data structure to retrieve informations about a node representing
		// an instruction.
		std::map<Instruction*, std::pair<unsigned int, GraphNode*>> instrToNode;

		ProgramDependenceGraph(std::string _functionName) 
			: functionName(_functionName), nextInstrID(0) {}

		void addNode(Instruction* ins);

		void addEntryNode(Instruction* ins);

		void addExitNode(Instruction* ins);

		void addEdge(Instruction* srcI, Instruction* dstI, DependenceType type);

		void connectToEntry(Instruction* dstI);

		void connectToExit(Instruction* srcI);

		void dumpToDot(Function& F);

		int tarjanVisit(GraphNode* v,
                       std::map<GraphNode*, std::pair<int, int> >* indexLowLink,
                       std::stack<GraphNode*>* stack,
                       std::map<GraphNode*, bool>*onStack,
                       std::set<std::set<GraphNode*> >*SCCs, int index);

		std::set<std::set<GraphNode*> > findStrongConnectedComponents();

		void getSubgraphOnNode(GraphNode* node, 
			std::set<GraphNode*>& subgraph);

		void getSubgraphOnSCC(const std::set<GraphNode*>& scc, 
			std::set<GraphNode*>& subgraph);

		bool isSubSetOf(const std::set<GraphNode*> &subset, 
			const std::set<GraphNode*> &superset);

};

#endif
