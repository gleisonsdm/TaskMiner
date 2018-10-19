#include "ProgramDependenceGraph.h"


std::string GraphEdge::edgeLabel()
{
	switch (type)
	{
		case RAR: return "RAR";
		case RAWLC: return "RAW*";
		case WAW: return "WAW";
		case RAW: return "RAW";
		case WAR: return "WAR";
		case CTR: return "CTR";
		case SCA:
		{
			if (src->instr->hasName())
				return src->instr->getName();
			else
				return "SCA";
		}
		default: return std::to_string(type);
	}
}

GraphNode* ProgramDependenceGraph::getGraphNode(Instruction* instr) {
	if (instrToNode.find(instr) == instrToNode.end())  {
		instrToNode[instr] = std::make_pair(nextInstrID, new GraphNode(instr));
		nextInstrID++;
	}

	return instrToNode[instr].second;
}

void ProgramDependenceGraph::addNode(Instruction* ins) {
	getGraphNode(ins);
}

void ProgramDependenceGraph::addEntryNode(Instruction* ins) {
	this->entry = getGraphNode(ins);
}

void ProgramDependenceGraph::addExitNode(Instruction* ins) {
	this->exit = getGraphNode(ins);
}

void ProgramDependenceGraph::addEdge(Instruction* srcI, Instruction* dstI, DependenceType type) {
	auto src = getGraphNode(srcI);	
	auto dst = getGraphNode(dstI);	
	auto edge = new GraphEdge(src, dst, type);

	outEdges[src].insert(edge);
	inEdges[dst].insert(edge);
}


void ProgramDependenceGraph::connectToEntry(Instruction* dstI) {
	addEdge(entry->instr, dstI, DependenceType::CTR);
}

void ProgramDependenceGraph::connectToExit(Instruction* srcI) {
	addEdge(srcI, exit->instr, DependenceType::CTR);
}

void ProgramDependenceGraph::dumpToDot(Function& F) {
	errs() << "Dumping instructions for function :: " << F.getName() << "\n";

	// Dump the function source with instruction IDs for reference. We also
	// assign for each instruction address a smaller numerical ID for easy
	// visual inspection of the graph.
	for (Function::iterator bbIt = F.begin(), e = F.end(); bbIt != e; ++bbIt) {
		if (bbIt->hasName())
			errs() << bbIt->getName() << "\n";
		else
			errs() << "Unnamed Basic Block\n";

		for (BasicBlock::iterator insIt = bbIt->begin(), e = bbIt->end(); insIt != e; ++insIt) {
			errs() << "[" << instrToNode[&*insIt].first << "]" << *insIt << "\n";
		}
	}

	// Write the graph to a DOT file
	std::string graphName = functionName + ".dot";
	std::ofstream dotStream;
	dotStream.open(graphName);

	if (!dotStream.is_open()) {
		errs() << "Problem opening DOT file: " << graphName << "\n";
	}	
	else {
		dotStream << "digraph g {\n";

		// Create all nodes in DOT format
		for (auto node : instrToNode) {
			if (node.second.second == this->entry) 
				dotStream << "\t\"" << instrToNode[node.second.second->instr].first << "\" [label=entry];\n";
			else if (node.second.second == this->exit)
				dotStream << "\t\"" << instrToNode[node.second.second->instr].first << "\" [label=exit];\n";
			else
				dotStream << "\t\"" << instrToNode[node.second.second->instr].first << "\" [label=\"" << instrToNode[node.second.second->instr].first << ": " << (node.second.second->instr->getOpcodeName()) << "\"];\n";
		}

		dotStream << "\n\n";

		// Now print all outgoing edges and their labels
		for (auto src : outEdges) {
			for (auto dst : src.second) {
				if (dst->type == CTR)
					dotStream << "\t\"" << instrToNode[src.first->instr].first << "\" -> \"" << instrToNode[dst->dst->instr].first << "\" [style=dotted];\n";
				else
					dotStream << "\t\"" << instrToNode[src.first->instr].first << "\" -> \"" << instrToNode[dst->dst->instr].first << "\" [label=\"" << dst->edgeLabel() << "\"];\n";
			}

			dotStream << "\n";
		}

		auto sccs = findStrongConnectedComponents();

		//print SCCS
	  std::set<GraphNode *> q;
	  GraphNode *node;
		int i=0;
		for (std::set<std::set<GraphNode *>>::iterator it = sccs.begin();
       it != sccs.end(); ++it)
	  {
	    dotStream << "subgraph cluster_" << i << " {\n"
	              << " color=red4;"
	              << " label=SCC" << i + 1 << ";"
	              << " fillcolor=paleturquoise1;"
	              << " style=filled;\n";
	    q = (*it);
	    if (q.size() > 1)
	    {
	      std::set<GraphNode *>::iterator s = q.begin();
	      do
	      {
	        node = *s;
	        dotStream << instrToNode[node->instr].first;
	        s++;
	        if (s != q.end())
	        {
	          dotStream << ",";
	        }
	      } while (s != q.end());
	      dotStream << ";";
	    }

	    dotStream << "\n} ";
	    i++;
	  }


		dotStream << "}";
		dotStream.close();
	}

	errs() << "\n\n";
}

// This method implements the TARJAN's algorithm for finding SCC's
std::set<std::set<GraphNode *>> ProgramDependenceGraph::findStrongConnectedComponents()
{
  // This map stores the INDEX and the LOWLINK for each node.
  //->first : index
  //->second : lowlink
  std::map<GraphNode *, std::pair<int, int>> indexLowLink;

  // This set stores nodes that are in the set
  std::map<GraphNode *, bool> onStack;

  // Stack that stores nodes in each visit.
  std::stack<GraphNode *> stack;

  // Here we'll store the strong connected components
  std::set<std::set<GraphNode *>> SCCs;

  int maxIndex = 0;

  GraphNode* node;

  // Initialize all indexes and lowlinks on -1;
  for (auto &it : instrToNode)
  {
		node = it.second.second;
    indexLowLink.insert(std::pair<GraphNode *, std::pair<int, int>>(
        (node), std::pair<int, int>(-1, -1)));
    onStack.insert(std::pair<GraphNode *, bool>((node), false));
  }

  // call tarjan's algorithm for each node if node hasn't been visited yet
  for (auto &it : instrToNode)
  {
  	node = it.second.second;
    if (indexLowLink[(node)].first == -1)
    {
      maxIndex = tarjanVisit((node), &indexLowLink, &stack, &onStack, &SCCs,
                               maxIndex);
    }
  }

  return SCCs;
}

int ProgramDependenceGraph::tarjanVisit(GraphNode *v,
                       std::map<GraphNode *, std::pair<int, int>> *indexLowLink,
                       std::stack<GraphNode *> *stack,
                       std::map<GraphNode *, bool> *onStack,
                       std::set<std::set<GraphNode *>> *SCCs, int index)
{
  int maxIndex;
  GraphNode *w;

  // index
  (*indexLowLink)[v].first = index;
  // lowlink
  (*indexLowLink)[v].second = index;

  maxIndex = index + 1;
  stack->push(v);
  (*onStack)[v] = true;

  //get successors of node V
  std::set<GraphEdge*> succs = outEdges[v];

  for (auto &succ : succs)
  {
      w = succ->dst;
      if ((*indexLowLink)[w].first == -1)
      {
        maxIndex = tarjanVisit(w, indexLowLink, stack, onStack, SCCs, maxIndex);
        (*indexLowLink)[v].second =
            std::min((*indexLowLink)[v].second, (*indexLowLink)[w].second);
      }
      else if ((*onStack)[w])
      {
        (*indexLowLink)[v].second =
            std::min((*indexLowLink)[v].second, (*indexLowLink)[w].first);
      }
  }

  // if v is a root node, pop the stack and generate an SCC
  if ((*indexLowLink)[v].second == (*indexLowLink)[v].first)
  {
    std::set<GraphNode *> newSCC;
    do
    {
      w = stack->top();
      stack->pop();
      (*onStack)[w] = false;
      newSCC.insert(w);
    } while (w != v);
    SCCs->insert(newSCC);
  }

  return maxIndex;
}

void ProgramDependenceGraph::getSubgraphOnNode(GraphNode* node, 
	std::set<GraphNode*>& subgraph)
{
	for (auto e : outEdges[node])
	{		
		if (subgraph.find(e->dst) != subgraph.end())
		{
			if (e->dst != exit)
				subgraph.insert(e->dst);
			getSubgraphOnNode(e->dst, subgraph);
		}
	}
}

void ProgramDependenceGraph::getSubgraphOnSCC(const std::set<GraphNode*>& scc, 
	std::set<GraphNode*>& subgraph)
{
	std::list<GraphEdge*> sccOutEdges;

	for (auto n : scc)
	{
		for (auto e : outEdges[n])
		{
			if (scc.find(e->dst) == scc.end())
			{
				sccOutEdges.push_back(e);
			}
		}
	}

	for (auto out_ : sccOutEdges)
	{
		if (out_->dst != exit)
			subgraph.insert(out_->dst);
		(out_->dst, subgraph);
	}

	return;
}

bool ProgramDependenceGraph::isSubSetOf(const std::set<GraphNode*> &subset, 
	const std::set<GraphNode*> &superset)
{
	bool isSubset = true;
	for (const auto &node : subset)
	{
		if (std::find(superset.begin(), superset.end(), node) == std::end(superset))
		{
			isSubset = false;
			break;
		}
	}
	
	return isSubset;
}



