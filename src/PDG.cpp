//LOCAL IMPORTS
#include "PDG.h"

//LLVM IMPORTS
#include "llvm/Support/raw_ostream.h"

//STL IMPORTS
#include <fstream>

using namespace llvm;

std::string PDG::edgeLabel(Edge<Instruction*, EdgeDepType> *e)
{
	switch (e->getType())
	{
		case EdgeDepType::RAR: return "RAR";
		case EdgeDepType::RAWLC: return "RAW*";
		case EdgeDepType::WAW: return "WAW";
		case EdgeDepType::RAW: return "RAW";
		case EdgeDepType::WAR: return "WAR";
		case EdgeDepType::CTR: return "CTR";
		case EdgeDepType::PARENT: return "PARENT";
		case EdgeDepType::SCA:
		{
			if (e->getSrc()->getItem()->hasName())
				return e->getSrc()->getItem()->getName();
			else
				return "SCA";
		}
		default: return std::to_string(e->getType());
	}
}

void PDG::dumpToDot()
{
	// errs() << "Dumping instructions for function :: " << F->getName() << "\n";
	
	// // Dump the function source with instruction IDs for reference. We also
	// // assign for each instruction address a smaller numerical ID for easy
	// // visual inspection of the graph.
	// for (Function::iterator bbIt = F->begin(), e = F->end(); bbIt != e; ++bbIt)
	// {
	// 	if (bbIt->hasName())
	// 		errs() << bbIt->getName() << "\n";
	// 	else
	// 		errs() << "Unnamed Basic Block\n";

	// 	for (BasicBlock::iterator insIt = bbIt->begin(), e = bbIt->end(); insIt != e; ++insIt)
	// 	{
	// 		errs() << "[" << getNodeIndex(&*insIt) << "]" << *insIt << "\n";
	// 	}]
	// }

	// Write the graph to a DOT file
	std::string graphName = functionName + ".dot";
	std::ofstream dotStream;
	dotStream.open(graphName);

	if (!dotStream.is_open())
	{
		errs() << "Problem opening DOT file: " << graphName << "\n";
	}	
	else
	{
		dotStream << "digraph g {\n";

		// Create all nodes in DOT format
		for (auto node : getNodes())
		{
			if (node == this->entry)
				dotStream << "\t\"" << getNodeIndex(node) << "\" [label=entry];\n";
			else if (node == this->exit)
				dotStream << "\t\"" << getNodeIndex(node) << "\" [label=exit];\n";
			else 
				dotStream << "\t\"" << getNodeIndex(node) << "\" [label=\"" << getNodeIndex(node) << ": " << node->getItem()->getOpcodeName() << "\"];\n";
		}

		dotStream << "\n\n";

		// Now print all outgoing edges and their labels
		for (auto e : getEdges())
		{
				if (e->getType() == EdgeDepType::CTR)
					dotStream << "\t\"" << getNodeIndex(e->getSrc()) << "\" -> \"" << getNodeIndex(e->getDst()) << "\" [style=dotted];\n";
				else
					dotStream << "\t\"" << getNodeIndex(e->getSrc()) << "\" -> \"" << getNodeIndex(e->getDst()) << "\" [label=\"" << edgeLabel(e) << "\"];\n";
			dotStream << "\n";
		}

		auto sccs = getStronglyConnectedSubgraphs();

		//print SCCS
	  std::list<Node<Instruction*>* > q;
	  Node<Instruction*> *node;
		int i=0;
		for (auto it = sccs.begin();
       it != sccs.end(); ++it)
	  {
	    dotStream << "subgraph cluster_" << i << " {\n"
	              << " color=red4;"
	              << " label=SCC" << i + 1 << ";"
	              << " fillcolor=paleturquoise1;"
	              << " style=filled;\n";
	    q = (*it)->getNodes();
	    if (q.size() > 1)
	    {
	      std::list<Node<Instruction*> *>::iterator s = q.begin();
	      do
	      {
	        node = *s;
	        dotStream << getNodeIndex(node->getItem());
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

void PDG::connectToEntry(Instruction* inst)
{
	auto n = getNode(inst);
	addEdge(entry, n, EdgeDepType::CTR);
}

void PDG::connectToExit(Instruction* inst)
{
	auto n = getNode(inst);
	addEdge(n, exit, EdgeDepType::CTR);
}

std::set<Graph<Instruction*, EdgeDepType>* > 
PDG::getStronglyConnectedSubgraphs()
{
	if (scSubgraphs.empty())
	{
		scSubgraphs = getStrongConnectedComponents();
	}
	return scSubgraphs;
}

