//===- IntraProc/ControlDependenceGraph.cpp ---------------------*- C++ -*-===//
//
//                      Static Program Analysis for LLVM
//
// This file is distributed under a Modified BSD License (see LICENSE.TXT).
//
//===----------------------------------------------------------------------===//
//
// This file defines the ControlDependenceGraph class, which allows fast and 
// efficient control dependence queries. It is based on Ferrante et al's "The 
// Program Dependence Graph and Its Use in Optimization."
//
//===----------------------------------------------------------------------===//
//LLVM IMPORTS
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/CFG.h"

//STL IMPORTS
#include <deque>
#include <set>

//LOCAL IMPORTS
#include "ControlDependenceGraph.h"
 
using namespace llvm;

namespace llvm {

void ControlDependenceNode::addTrue(ControlDependenceNode *Child) {
	TrueChildren.insert(Child);
}

void ControlDependenceNode::addFalse(ControlDependenceNode *Child) {
	FalseChildren.insert(Child);
}

void ControlDependenceNode::addOther(ControlDependenceNode *Child) {
	OtherChildren.insert(Child);
}

void ControlDependenceNode::addParent(ControlDependenceNode *Parent) {
	assert(std::find(Parent->begin(), Parent->end(), this) != Parent->end() && "Must be a child before adding the parent!");
	Parents.insert(Parent);
}

void ControlDependenceNode::removeTrue(ControlDependenceNode *Child) {
	node_iterator CN = TrueChildren.find(Child);

	if (CN != TrueChildren.end())
		TrueChildren.erase(CN);
}

void ControlDependenceNode::removeFalse(ControlDependenceNode *Child) {
	node_iterator CN = FalseChildren.find(Child);

	if (CN != FalseChildren.end())
		FalseChildren.erase(CN);
}

void ControlDependenceNode::removeOther(ControlDependenceNode *Child) {
	node_iterator CN = OtherChildren.find(Child);

	if (CN != OtherChildren.end())
		OtherChildren.erase(CN);
}

void ControlDependenceNode::removeParent(ControlDependenceNode *Parent) {
	node_iterator PN = Parents.find(Parent);

	if (PN != Parents.end())
		Parents.erase(PN);
}

ControlDependenceNode::EdgeType ControlDependenceGraphBase::getEdgeType(const BasicBlock *A, const BasicBlock *B) {
	if (const BranchInst *b = dyn_cast<BranchInst>(A->getTerminator())) {
		if (b->isConditional()) {
			if (b->getSuccessor(0) == B) {
				return ControlDependenceNode::TRUE;
			} 
			else if (b->getSuccessor(1) == B) {
				return ControlDependenceNode::FALSE;
			} 
			else {
				assert(false && "Asking for edge type between unconnected basic blocks!");
			}
		}
	}

	return ControlDependenceNode::OTHER;
}

void ControlDependenceGraphBase::computeDependencies(Function &F, PostDominatorTree &pdt) {
	root = new ControlDependenceNode();
	nodes.insert(root);

	for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB) {
		ControlDependenceNode *bn = new ControlDependenceNode(&*BB);
		nodes.insert(bn);
		bbMap[&*BB] = bn;
	}

	for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB) {
		BasicBlock *A = &*BB;
		ControlDependenceNode *AN = bbMap[A];

		for (succ_iterator succ = succ_begin(A), end = succ_end(A); succ != end; ++succ) {
			BasicBlock *B = *succ;
			assert(A && B);

			if (A == B || !pdt.dominates(B, A)) {
				BasicBlock *L = pdt.findNearestCommonDominator(A,B);

				ControlDependenceNode::EdgeType type = ControlDependenceGraphBase::getEdgeType(A,B);

				if (A == L) {
					switch (type) {
						case ControlDependenceNode::TRUE:
							AN->addTrue(AN); 
							break;
						case ControlDependenceNode::FALSE:
							AN->addFalse(AN); 
							break;
						case ControlDependenceNode::OTHER:
							AN->addOther(AN); 
							break;
					}

					AN->addParent(AN);
				}

				for (DomTreeNode *cur = pdt[B]; cur && cur != pdt[L]; cur = cur->getIDom()) {
					ControlDependenceNode *CN = bbMap[cur->getBlock()];

					switch (type) {
						case ControlDependenceNode::TRUE:
							AN->addTrue(CN); 
							break;
						case ControlDependenceNode::FALSE:
							AN->addFalse(CN); 
							break;
						case ControlDependenceNode::OTHER:
							AN->addOther(CN); 
							break;
					}

					assert(CN);
					CN->addParent(AN);
				}
			}
		}
	}

	// ENTRY -> START
	for (DomTreeNode *cur = pdt[&F.getEntryBlock()]; cur; cur = cur->getIDom()) {
		if (cur->getBlock()) {
			ControlDependenceNode *CN = bbMap[cur->getBlock()];
			assert(CN);
			root->addOther(CN); CN->addParent(root);
		}
	}
}

void ControlDependenceGraphBase::graphForFunction(Function &F, PostDominatorTree &pdt) {
	computeDependencies(F,pdt);
}

bool ControlDependenceGraphBase::controls(BasicBlock *A, BasicBlock *B) const {
	const ControlDependenceNode *n = getNode(B);

	assert(n && "Basic block not in control dependence graph!");

	while (n->getNumParents() == 1) {
		n = *n->parent_begin();

		if (n->getBlock() == A)
			return true;
	}

	return false;
}

bool ControlDependenceGraphBase::influences(BasicBlock *A, BasicBlock *B) const {
	const ControlDependenceNode *n = getNode(B);

	assert(n && "Basic block not in control dependence graph!");

	std::deque<ControlDependenceNode *> worklist;
	worklist.insert(worklist.end(), n->parent_begin(), n->parent_end());

	while (!worklist.empty()) {
		n = worklist.front();
		worklist.pop_front();

		if (n->getBlock() == A) 
			return true;

		worklist.insert(worklist.end(), n->parent_begin(), n->parent_end());
	}

	return false;
}

} // namespace llvm

