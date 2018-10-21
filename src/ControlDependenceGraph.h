#ifndef ANALYSIS_CONTROLDEPENDENCEGRAPH_H
#define ANALYSIS_CONTROLDEPENDENCEGRAPH_H

//LLVM IMPORTS
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"

//STL IMPORTS
#include <map>
#include <set>
#include <iterator>

namespace llvm {

class BasicBlock;
class ControlDependenceGraphBase;

class ControlDependenceNode {
public:
	enum EdgeType { TRUE, FALSE, OTHER };
	typedef std::set<ControlDependenceNode *>::iterator       node_iterator;
	typedef std::set<ControlDependenceNode *>::const_iterator const_node_iterator;

	struct edge_iterator {
		typedef node_iterator::value_type      value_type;
		typedef node_iterator::difference_type difference_type;
		typedef node_iterator::reference       reference;
		typedef node_iterator::pointer         pointer;
		typedef std::input_iterator_tag        iterator_category;

		edge_iterator(ControlDependenceNode *n) : 
			node(n), stage(TRUE), it(n->TrueChildren.begin()), end(n->TrueChildren.end()) {
			while ((stage != OTHER) && (it == end)) this->operator++();
			}

			edge_iterator(ControlDependenceNode *n, EdgeType t, node_iterator i, node_iterator e) :
				node(n), stage(t), it(i), end(e) {
				while ((stage != OTHER) && (it == end)) 
					this->operator++();
			}

			EdgeType type() const { return stage; }

			bool operator==(edge_iterator const &other) const { 
				return (this->stage == other.stage) && (this->it == other.it);
			}

		bool operator!=(edge_iterator const &other) const { return !(*this == other); }

		reference operator*()  { return *this->it; }

		pointer   operator->() { return &*this->it; }

		edge_iterator& operator++() {
			if (it != end) ++it;

			while ((stage != OTHER) && (it == end)) {
				if (stage == TRUE) {
					it = node->FalseChildren.begin();
					end = node->FalseChildren.end();
					stage = FALSE;
				} 
				else {
					it = node->OtherChildren.begin();
					end = node->OtherChildren.end();
					stage = OTHER;
				}
			}
			return *this;
		}

		edge_iterator operator++(int) {
			edge_iterator ret(*this);
			assert(ret.stage == OTHER || ret.it != ret.end);
			this->operator++();
			return ret;
		}

		private:
			ControlDependenceNode *node;
			EdgeType stage;
			node_iterator it, end;
	};

	edge_iterator begin() { return edge_iterator(this); }
	edge_iterator end()   { return edge_iterator(this, OTHER, OtherChildren.end(), OtherChildren.end()); }

	node_iterator true_begin()   { return TrueChildren.begin(); }
	node_iterator true_end()     { return TrueChildren.end(); }

	node_iterator false_begin()  { return FalseChildren.begin(); }
	node_iterator false_end()    { return FalseChildren.end(); }

	node_iterator other_begin()  { return OtherChildren.begin(); }
	node_iterator other_end()    { return OtherChildren.end(); }

	node_iterator parent_begin() { return Parents.begin(); }
	node_iterator parent_end()   { return Parents.end(); }

	const_node_iterator parent_begin() const { return Parents.begin(); }
	const_node_iterator parent_end()   const { return Parents.end(); }

	BasicBlock *getBlock() const { return TheBB; }

	size_t getNumParents() const { return Parents.size(); }

	size_t getNumChildren() const { 
		return TrueChildren.size() + FalseChildren.size() + OtherChildren.size();
	}


private:
	BasicBlock *TheBB;
	std::set<ControlDependenceNode *> Parents;
	std::set<ControlDependenceNode *> TrueChildren;
	std::set<ControlDependenceNode *> FalseChildren;
	std::set<ControlDependenceNode *> OtherChildren;

	friend class ControlDependenceGraphBase;

	void clearAllChildren() {
		TrueChildren.clear();
		FalseChildren.clear();
		OtherChildren.clear();
	}

	void clearAllParents() { Parents.clear(); }

	void addTrue(ControlDependenceNode *Child);
	void addFalse(ControlDependenceNode *Child);
	void addOther(ControlDependenceNode *Child);
	void addParent(ControlDependenceNode *Parent);
	void removeTrue(ControlDependenceNode *Child);
	void removeFalse(ControlDependenceNode *Child);
	void removeOther(ControlDependenceNode *Child);
	void removeParent(ControlDependenceNode *Child);

	ControlDependenceNode() : TheBB(NULL) {}
	ControlDependenceNode(BasicBlock *bb) : TheBB(bb) {}
};
  
class ControlDependenceGraphBase {
public:
	ControlDependenceGraphBase() : root(NULL) {}

	virtual ~ControlDependenceGraphBase() { releaseMemory(); }

	virtual void releaseMemory() {
		for (ControlDependenceNode::node_iterator n = nodes.begin(), e = nodes.end(); n != e; ++n) 
			delete *n;

		nodes.clear();
		bbMap.clear();
		root = NULL;
	}

	void graphForFunction(Function &F, PostDominatorTree &pdt);

	ControlDependenceNode *getRoot()             { return root; }

	const ControlDependenceNode *getRoot() const { return root; }

	ControlDependenceNode *operator[](const BasicBlock *BB)             { return getNode(BB); }

	const ControlDependenceNode *operator[](const BasicBlock *BB) const { return getNode(BB); }

	ControlDependenceNode *getNode(const BasicBlock *BB) { 
		return bbMap[BB];
	}

	const ControlDependenceNode *getNode(const BasicBlock *BB) const {
		return (bbMap.find(BB) != bbMap.end()) ? bbMap.find(BB)->second : NULL;
	}

	bool isDominated(BasicBlock* BB) {
		ControlDependenceNode* GN = getNode(BB);

		if (GN->getNumParents() == 0) return false;

		auto PABeg = GN->parent_begin();
		auto PAEnd = GN->parent_end();

		while (PABeg != PAEnd) {
			if (*PABeg == this->root) return false;

			PABeg++;
		}

		return true;
	}

	bool controls(BasicBlock *A, BasicBlock *B) const;
	bool influences(BasicBlock *A, BasicBlock *B) const;

private:
	ControlDependenceNode *root;
	std::set<ControlDependenceNode *> nodes;
	std::map<const BasicBlock *,ControlDependenceNode *> bbMap;
	static ControlDependenceNode::EdgeType getEdgeType(const BasicBlock *, const BasicBlock *);
	void computeDependencies(Function &F, PostDominatorTree &pdt);
};

} // namespace llvm

#endif // ANALYSIS_CONTROLDEPENDENCEGRAPH_H
