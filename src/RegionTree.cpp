//LOCAL IMPORTS
#include "RegionTree.h"

//LLVM IMPORTS
#include "llvm/Support/raw_ostream.h"

//STL IMPORTS
#include <fstream>

using namespace llvm;

std::string RegionTree::edgeLabel(Edge<RegionWrapper*, EdgeDepType> *e) const
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
		case EdgeDepType::RECURSIVE: return "RECURSIVE";
		case EdgeDepType::FCALL: return "FCALL";
		case EdgeDepType::SCA: return "SCA";
		default: llvm_unreachable("EdgeType not found.");
;
	}
}

void RegionTree::dumpToDot(std::string filename, bool dep)
{
	// Write the graph to a DOT file
	std::string graphName = filename + "_RT.dot";
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
			dotStream << "\t\"" << getNodeIndex(node) << "\" [label=\"R" << getNodeIndex(node) << "\"];\n";
		}

		dotStream << "\n\n";

		// Now print all outgoing edges and their labels
		if (dep)
			for (auto e : getEdges())
			{
				int intensity = e->getIntensity();
				if (intensity > 10)
					intensity = 10;

					if (e->getType() == EdgeDepType::CTR)
						dotStream << "\t\"" << getNodeIndex(e->getSrc()) << "\" -> \"" << getNodeIndex(e->getDst()) << "\" [style=dotted];\n";
					else if (e->getType() == EdgeDepType::RAW)
						dotStream << "\t\"" << getNodeIndex(e->getSrc()) << "\" -> \"" << getNodeIndex(e->getDst()) << "\" [label=\"" << edgeLabel(e) << "\" color=\"blue\" penwidth ="<< intensity << "];\n";
					else if (e->getType() == EdgeDepType::WAR)
						dotStream << "\t\"" << getNodeIndex(e->getSrc()) << "\" -> \"" << getNodeIndex(e->getDst()) << "\" [label=\"" << edgeLabel(e) << "\" color=\"green\" penwidth ="<< intensity << "];\n";
					else if (e->getType() == EdgeDepType::RAR)
						dotStream << "\t\"" << getNodeIndex(e->getSrc()) << "\" -> \"" << getNodeIndex(e->getDst()) << "\" [label=\"" << edgeLabel(e) << "\" color=\"yellow\" penwidth ="<< intensity << "];\n";
					else if (e->getType() == EdgeDepType::WAW)
						dotStream << "\t\"" << getNodeIndex(e->getSrc()) << "\" -> \"" << getNodeIndex(e->getDst()) << "\" [label=\"" << edgeLabel(e) << "\" color=\"gray\" penwidth ="<< intensity << "];\n";
					else if (e->getType() == EdgeDepType::RAWLC)
						dotStream << "\t\"" << getNodeIndex(e->getSrc()) << "\" -> \"" << getNodeIndex(e->getDst()) << "\" [label=\"" << edgeLabel(e) << "\" color=\"red\" penwidth ="<< intensity << "];\n";
					else if (e->getType() == EdgeDepType::SCA)
						dotStream << "\t\"" << getNodeIndex(e->getSrc()) << "\" -> \"" << getNodeIndex(e->getDst()) << "\" [label=\"" << edgeLabel(e) << "\" color=\"orange\" penwidth ="<< intensity << "];\n";
					else if (e->getType() == EdgeDepType::FCALL)
						dotStream << "\t\"" << getNodeIndex(e->getSrc()) << "\" -> \"" << getNodeIndex(e->getDst()) << "\" [label=\"" << edgeLabel(e) << "\" style=dashed color=\"black\"];\n";
					else
						dotStream << "\t\"" << getNodeIndex(e->getSrc()) << "\" -> \"" << getNodeIndex(e->getDst()) << "\" [label=\"" << edgeLabel(e) << "\" color=\"black\" penwidth =1];\n";
				dotStream << "\n";
			}

		// //Draw parent edges
		// Region *parent;
		// for (auto n : getNodes())
		// {
		// 	if ((parent = n->getItem()->getParent()))
		// 	{
		// 		dotStream << "\t\"" << getNodeIndex(parent) << "\" -> \"" << getNodeIndex(n) << "\" [label=\"PARENT\" color=\"black\" penwidth =1];\n";
		// 	}
		// }



		auto sccs = getStronglyConnectedSubgraphs();

		//print SCCS
	  std::list<Node<RegionWrapper*>* > q;
	  Node<RegionWrapper*> *node;
		int i=1;
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
	      std::list<Node<RegionWrapper*> *>::iterator s = q.begin();
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

		// //Legend table
		// dotStream << "\nnode[shape=plaintext]"   
		// 					<< "subgraph cluster_00 {\n"
  //  						<< "label = \"Legend\";\n"
  //   					<< "key [label=<<table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" cellborder=\"0\">\n"
		// 					<< "<tr><td align=\"right\" port=\"i1\">CTR</td></tr>\n"
  //     				<< "<tr><td align=\"right\" port=\"i2\">RAW</td></tr>\n"
		// 					<< "<tr><td align=\"right\" port=\"i3\">WAR</td></tr>\n"
		// 					<< "<tr><td align=\"right\" port=\"i4\">RAR</td></tr>\n"
		// 					<< "<tr><td align=\"right\" port=\"i5\">WAW</td></tr>\n"
		// 					<< "<tr><td align=\"right\" port=\"i6\">Loop-carried RAW</td></tr>\n"
		// 					<< "<tr><td align=\"right\" port=\"i7\">SCA</td></tr>\n"
		// 					<< "</table>>]\n"
		// 					<< "key2 [label=<<table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" cellborder=\"0\">\n"
		// 					<< "<tr><td port=\"i1\">&nbsp;</td></tr>\n"
		// 					<< "<tr><td port=\"i2\">&nbsp;</td></tr>\n"
		// 					<< "<tr><td port=\"i3\">&nbsp;</td></tr>\n"
		// 					<< "<tr><td port=\"i4\">&nbsp;</td></tr>\n"
		// 					<< "<tr><td port=\"i5\">&nbsp;</td></tr>\n"
		// 					<< "<tr><td port=\"i6\">&nbsp;</td></tr>\n"
		// 					<< "<tr><td port=\"i7\">&nbsp;</td></tr>\n"
		// 					<< "</table>>]\n"
		// 					<< "key:i1:e -> key2:i1:w [style=dashed]\n"
		// 					<< "key:i2:e -> key2:i2:w [color=red]\n"
		// 					<< "key:i3:e -> key2:i3:w [color=blue]\n"
		// 					<< "key:i4:e -> key2:i4:w [color=yellow]\n"
		// 					<< "key:i5:e -> key2:i5:w [color=gray]\n"
		// 					<< "key:i6:e -> key2:i6:w [color=green]\n"
		// 					<< "key:i7:e -> key2:i7:w [color=orange] }\n\n\n\n";


		dotStream << "}";
		dotStream.close();
	}

	// errs() << "\n\n";
}

std::set<Graph<RegionWrapper*, EdgeDepType>* > 
RegionTree::getStronglyConnectedSubgraphs()
{
	if (scSubgraphs.empty())
	{
		scSubgraphs = getStrongConnectedComponents();
	}
	return scSubgraphs;
}

Node<RegionWrapper*>* 
RegionTree::getNode(BasicBlock* entry, BasicBlock* exit, bool topLevel, bool isHub)
{
	for (auto n : getNodes())
	{
		auto rw = n->getItem();
		if ((!topLevel) 
					&& ((rw->entry == entry)
					&& (rw->exit == exit)
					&& (rw->isHub == isHub)))
		{
			return n;			
		}
		else if ((topLevel) 
							&& (rw->entry == entry)
							&& (rw->isHub == isHub))
		{
			return n;
		}
	}

	return nullptr;
}

Node<RegionWrapper*>* 
RegionTree::getNode(RegionWrapper* rw)
{
	for (auto n : getNodes())
	{
		if (n->getItem() == rw)
			return n;
	}

	return nullptr;
}

Node<RegionWrapper*> *RegionTree::getTopLevelRegionNode(Function *F, bool isHub)
{
	//ATTENTION: WHEN ISHUB == TRUE, THERE MIGHT BE MORE THAN ONE CANDIDATE.
	for (auto n : getNodes())
	{
		auto rw = n->getItem();
		if ((rw->F == F) && (rw->isHub == isHub) && (rw->topLevel))
		{
			return n;
		}
	}
	return nullptr;
}

RegionTree* RegionTree::copyRegionTree(RegionTree* src)
{
	RegionTree* rt_copy = new RegionTree();
	std::map<Node<RegionWrapper*>*, Node<RegionWrapper*>*> originalToHub;

	//Copy nodes
	for (auto n : src->getNodes())
	{
		RegionWrapper* original = n->getItem();
		RegionWrapper* rw_hub = copyRegionWrapper(original);
		originalToHub[n] = rt_copy->addNode(rw_hub);
	}

	//Copy edges
	for (auto e : src->getEdges())
	{
		auto srcOriginal = e->getSrc();
		auto dstOriginal = e->getDst();
		auto type = e->getType();
		Edge<RegionWrapper*, EdgeDepType> *e_;

		if (type == EdgeDepType::FCALL)
			continue;

		auto srcHub = originalToHub[srcOriginal];
		auto dstHub = originalToHub[dstOriginal];

		if (type == EdgeDepType::RECURSIVE)
		{
			//CHECK IF RECURSIVE EDGE EXISTS BECAUSE WE WON'T ADD THEM TWICE TO THE HUB
			if (!(e_ = checkEdge(srcHub, dstHub, type)))
			{
				rt_copy->addEdge(srcHub, dstHub, type);
			}
		}
		else
		{
			//Now add all the other edges
			rt_copy->addEdge(srcHub, dstHub, type);	
		}
		
	}

	return rt_copy;
}

RegionWrapper* RegionTree::copyRegionWrapper(RegionWrapper* src)
{
	RegionWrapper* copy = new RegionWrapper();

	copy->entry = src->entry;
	copy->exit = src->exit;
	copy->F = src->F;
	copy->L = src->L;
	copy->isHub = true;
	copy->topLevel = src->topLevel;
	copy->hasLoop = src->hasLoop;

	return copy;
}

bool RegionTree::compareTwoRegionWrappers(RegionWrapper* A, RegionWrapper* B)
{
	if (A->isHub || A->F != B->F)
		return false;
	else if (A->topLevel && A->entry == B->entry)
		return true;
	else if (A->entry == B->entry && A->exit == B->exit)
		return true;
	else
		return false;
}

raw_ostream& RegionTree::print(raw_ostream& os) const
{
	os << "\nPrinting Region Tree\n";
	os << "====\nNodes:\n==== ";
	for (auto n : getNodes())
	{
		if (RegionWrapper* R = dyn_cast<RegionWrapper>(n->getItem()))
		{
			if (R->isHub)
			{
				errs() << "<<HUB REGION>>";
			}
			if (R->getEntry())
				os << "\n" << R->getEntry()->getName();
			if (!R->topLevel)
				os << " -> " << R->exit->getName();
			else
				os << " [TOPLEVEL REGION]";
			os << " at FUNCTION: " << R->F->getName();
		}
	}
	os << "\n====\nEdges:\n==== ";
	for (auto e : getEdges())
	{
		if (RegionWrapper *R = dyn_cast<RegionWrapper>(e->getSrc()->getItem()))
		{
			if (R->isHub)
			{
				errs() << "<<HUB REGION>>";
			}
			if (R->getEntry())
				os << "\n" << R->getEntry()->getName();
			if (!R->topLevel)
				os << " -> " << R->exit->getName();
			else
				os << " [TOPLEVEL REGION]";
			os << " at FUNCTION: " << R->F->getName();
		}
		os << "\n     ->" << edgeLabel(e) << " to: ";
		if (RegionWrapper *R = dyn_cast<RegionWrapper>(e->getDst()->getItem()))
		{
			if (R->isHub)
			{
				errs() << "<<HUB REGION>>";
			}
			if (R->getEntry())
				os << "\n" << R->getEntry()->getName();
			if (!R->topLevel)
				os << " -> " << R->exit->getName();
			else
				os << " [TOPLEVEL REGION]";
			os << " at FUNCTION: " << R->F->getName();
		}

		os << "\n";
	}
}