#ifndef GRAPH_HPP
#define GRAPH_HPP
// #define DEBUG_GRAPH_HPP

//STL IMPORTS
#include <map>
#include <vector>
#include <set>
#include <stack>
#include <list>
#include <iostream>
#include <utility>
#include <algorithm>
#include <iterator>

template<typename NodeT>
class Node
{
private:
	NodeT item;
public:
	Node(NodeT _item) 
		: item(_item)
		{}
	~Node() {};

	NodeT getItem() const { return item; }
};

template<typename NodeT, typename EdgeT>
class Edge
{
private:
	Node<NodeT> *src;
	Node<NodeT> *dst;
	EdgeT type;
	int intensity;

public:
	Edge(Node<NodeT> *_src, Node<NodeT> *_dst, EdgeT _type)
		: src(_src)
		, dst(_dst)
		, type(_type)
		, intensity(1) 
		{}
	~Edge() {};

	Node<NodeT> *getSrc() const { return src; }
	Node<NodeT> *getDst() const { return dst; }
	EdgeT getType() const { return type; }
	EdgeT operator+(Edge<NodeT, EdgeT> &other)
	{
		return this->type + other.type;
	}
	void addIntensity() { intensity++; }
	int getIntensity() { return intensity; }
};

template<typename NodeT, typename EdgeT>
class Graph
{
private:
	unsigned nextIntKey = 0;
	//This stores a map from object of type T to it's respective pair (Key, Node)
	std::map<NodeT, std::pair<int, Node<NodeT>* > > nodes;
	std::list<Node<NodeT>* > nodesList;
	std::list<Edge<NodeT, EdgeT>* > edgesList;
	//This map stores all the outcoming edges from node of type T
	std::map<Node<NodeT>*, std::set<Edge<NodeT, EdgeT>*> > outEdges;
	//This map stores all the incoming edges to node of type T
	std::map<Node<NodeT>*, std::set<Edge<NodeT, EdgeT>*> > inEdges;
	bool stronglyConnected=true;

	bool nodeReachesEveryNode(Node<NodeT> *node)
	{
		std::map<const Node<NodeT> *, bool> visited;
		for (const auto n : nodesList)
			visited[n] = false;
		visit(node, visited);
		for (const auto v : visited)
		{
			if (v.second == false)
			{
				return false;				
			}
		}

		return true;
	}

	void visit(Node<NodeT> *node, std::map<const Node<NodeT>*, bool> &visited)
	{
		if (!visited[node])
		{
			visited[node] = true;
			for (const auto &e : getOutEdges(node))
			{
				visit(e->getDst(), visited);
			}			
		}
		else return;
	}

	void updateStronglyConnectedStatus()
	{
		Node<NodeT> *node = nodesList.front();
		stronglyConnected = nodeReachesEveryNode(node);
	}

public:
	Graph() {};
	~Graph()
	{
		for (auto &n : nodesList) delete n;
		for (auto &e : edgesList) delete e;
	}

	Node<NodeT> *operator[](NodeT item) const { return getNode(item); }

	Node<NodeT> *addNode(NodeT item)
	{
		if (nodes.find(item) == nodes.end())
		{
			Node<NodeT> *node = new Node<NodeT>(item);
			nodes[item] = std::make_pair<int,  Node<NodeT>* >(nextIntKey, std::move(node));
			nodesList.push_back(node);
			nextIntKey++;
			return node;
		}
		else
		{
			#ifdef DEBUG_GRAPH_HPP
				std::cout << "\nTrying to add an already added item.\n";
			#endif
			return nodes.find(item)->second.second;
		}
	}

	Node<NodeT> *getNode(NodeT item)
	{
		auto pair_ = nodes.find(item);
		if (pair_ == std::end(nodes))
			return addNode(item);
		else
			return pair_->second.second;
	}

	Node<NodeT> *getNodeByIndex(const int index) const
	{
		for (const auto &pair_ : nodes)
		{
			if (pair_.second.first == index)
			{
				return pair_.second.second;
			}
		}
		return nullptr;	
	}

	int getNodeIndex(NodeT item) const
	{
		auto pair_ = nodes.find(item);
		if (pair_ == nodes.end())
			return -1;
		return pair_->second.first;
	}

	int getNodeIndex(Node<NodeT> *node) const
	{
		for (auto &n : nodes)
		{
			if (n.second.second == node)
				return n.second.first;
		}
		return -1;
	}

	std::list< Node<NodeT>*> getNodes() const
	{
		return nodesList;
	}

	Edge<NodeT, EdgeT> *addEdge(Node<NodeT> *src, Node<NodeT> *dst, EdgeT e)
	{
		Edge<NodeT, EdgeT> *edge = new Edge<NodeT, EdgeT>(src, dst, e);
		outEdges[src].insert(edge);
		inEdges[dst].insert(edge);
		edgesList.push_back(edge);
		updateStronglyConnectedStatus();

		return edge;
	}

	Edge<NodeT, EdgeT> *addEdge(NodeT src, NodeT dst, EdgeT e)
	{
		Node<NodeT> *src_ = getNode(src);
		Node<NodeT> *dst_ = getNode(dst);

		return addEdge(src_, dst_, e);
	}

	std::set<Edge<NodeT, EdgeT>*> getInEdges(Node<NodeT> *node) 
	{
		std::set<Edge<NodeT, EdgeT>*> inEdges_;
		if (inEdges.find(node) != inEdges.end())
			inEdges_ = inEdges[node];
		
		return inEdges_;		
	}

	std::set<Edge<NodeT, EdgeT>*> getInEdges(NodeT item)
	{
		Node<NodeT> *node = getNode(item);
		
		return getInEdges(node);
	}

	std::set<Edge<NodeT, EdgeT>*> getOutEdges(Node<NodeT> *node) 
	{
		std::set<Edge<NodeT, EdgeT>*> outEdges_;
		if (outEdges.find(node) != outEdges.end())
			outEdges_ = outEdges[node];
		
		return outEdges_;
	}

	std::set<Edge<NodeT, EdgeT>*> getOutEdges(NodeT item)
	{
		Node<NodeT> *node = getNode(item);
		
		return getOutEdges(node);
	}

	Edge<NodeT, EdgeT> *checkEdge(NodeT src, NodeT dst, EdgeT type) const
	{
		for (auto e : edgesList)
		{
			if ((e->getSrc()->getItem() == src) 
				&& (e->getDst()->getItem() == dst)
				&& (e->getType() == type))
				return e;
		}
		return nullptr;
	}

	Edge<NodeT, EdgeT> *checkEdge(Node<NodeT> *src, Node<NodeT> *dst, EdgeT type) const
	{
		for (auto e : edgesList)
		{
			if ((e->getSrc() == src) 
				&& (e->getDst() == dst)
				&& (e->getType() == type))
				return e;
		}
		return nullptr;
	}

	void removeEdge(Edge<NodeT, EdgeT>* e)
	{
		auto out = e->getSrc();
		auto in = e->getDst();
		edgesList.remove(e);
		outEdges[out].erase(e);
		inEdges[in].erase(e);
	}

	std::list<Edge<NodeT, EdgeT>* > getEdges() const
	{
		return edgesList;
	}

	bool isSubgraphOf(const Graph<NodeT, EdgeT> &supergraph) const
	{
		for (const auto &node : supergraph.getNodes())
		{
			if (nodesList.find(node) == nodesList.end())
				return false;
		}
		for (const auto &edge : supergraph.getEdges())
		{
			if (edgesList.find(edge) == edgesList.end())
				return false;
		}
		return true;
	}

	bool isStronglyConnected() const
	{
		return stronglyConnected;
	}

	int size() const { return nextIntKey; }

	Graph<NodeT, EdgeT> *getSubgraphFromNode(Node<NodeT> *node)
	{
		Graph<NodeT, EdgeT> *subgraph = new Graph<NodeT, EdgeT>();

		fillSubgraphFromNodeRecursively(node, *subgraph);

		return subgraph;
	}

	bool nodeReachesSCC(NodeT item)
	{
		if (getNodeIndex(item) != -1)
		{
			auto sccs = getStrongConnectedComponents();
			std::set<Node<NodeT>* > visited;
			visitNeighbors(getNode(item), &visited);
			for (auto scc : sccs)
				for (auto node : scc->getNodes())
					for (auto v : visited)
						if (v->getItem() == node->getItem())
							return true;
		}

		return false;
	}

	void visitNeighbors(Node<NodeT> *node, std::set<Node<NodeT>*> *visited)
	{
		if (visited->find(node) != visited->end())
		{
			visited->insert(node);
			for (auto e : outEdges[node])
			{
				visitNeighbors(e->getDst(), visited);
			}
		}
	}

	void fillSubgraphFromNodeRecursively(Node<NodeT> *node, 
		Graph<NodeT, EdgeT> &subgraph)
	{
		std::vector<Node<NodeT>* > nodes(std::begin(subgraph.getNodes()), std::end(subgraph.getNodes()));
		if (std::find(nodes.begin(), nodes.end(), node) != nodes.end())
		{
			subgraph.addNode(node->getItem());
			// for (auto e : this->outEdges[node])
			// {
			// 	subgraph.addEdge(e->getSrc(), e->getDst(), e->getType());
			// }
			fillSubgraphFromNodeRecursively(node, subgraph);			
		}
	}

	std::set<Graph<NodeT, EdgeT>* > getStrongConnectedComponents()
	{
		// This map stores the INDEX and the LOWLINK for each node.
	  //->first : index
	  //->second : lowlink
	  std::map<Node<NodeT> *, std::pair<int, int> > indexLowLink;

	  // This set stores nodes that are in the set
	  std::map<Node<NodeT> *, bool> onStack;

	  // Stack that stores nodes in each visit.
	  std::stack<Node<NodeT> *> stack;

	  // Here we'll store the strong connected components
	  std::set<std::set<Node<NodeT> *> > SCCs;

	  int maxIndex = 0;
	  
	  // Initialize all indexes and lowlinks on -1;
	  for (auto &node : nodesList)
	  {
	    indexLowLink.insert(std::pair<Node<NodeT> *, std::pair<int, int> >(
	        (node), std::pair<int, int>(-1, -1)));
	    onStack.insert(std::pair<Node<NodeT> *, bool>((node), false));
	  }

	  // call tarjan's algorithm for each node if node hasn't been visited yet
	  for (auto &node: nodesList)
	  {
	    if (indexLowLink[(node)].first == -1)
	    {
	      maxIndex = tarjanVisit((node), &indexLowLink, &stack, &onStack, &SCCs,
	                               maxIndex);
	    }
	  }

	  //Run through the set of nodes for each SCC and build graphs
	  std::set<Graph<NodeT, EdgeT>* > subgraphs;
	  for (auto scc : SCCs)
	  {
	  	Graph<NodeT, EdgeT> *subgraph = new Graph<NodeT, EdgeT>();
	  	subgraphs.insert(subgraph);
	  	for (auto node : scc)
	  	{
	  		subgraph->addNode(node->getItem());
	  		for (auto e : outEdges[node])
	  		{
	  			if (scc.find(e->getDst()) != scc.end())
	  			{
	  				subgraph->addEdge(e->getSrc()->getItem(), e->getDst()->getItem(), e->getType());
	  			}
	  		}
	  	}
	  }

	  return subgraphs;
	}

	int tarjanVisit(Node<NodeT> *v,
                       std::map<Node<NodeT> *, std::pair<int, int> > *indexLowLink,
                       std::stack<Node<NodeT> *> *stack,
                       std::map<Node<NodeT> *, bool> *onStack,
                       std::set<std::set<Node<NodeT> *> > *SCCs, int index)
	{
	  int maxIndex;
	  Node<NodeT> *w;

	  // index
	  (*indexLowLink)[v].first = index;
	  // lowlink
	  (*indexLowLink)[v].second = index;

	  maxIndex = index + 1;
	  stack->push(v);
	  (*onStack)[v] = true;

	  //get successors of node V
	  std::set<Edge<NodeT, EdgeT> *> succs = outEdges[v];

	  for (auto &succ : succs)
	  {
	      w = succ->getDst();
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
	    std::set<Node<NodeT> *> newSCC;
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

	Graph<NodeT, EdgeT>* mergeWith(Graph<NodeT, EdgeT> *src)
	{
		Graph<NodeT, EdgeT> *dst = this;

		for (auto n : src->getNodes())
		{
			dst->addNode(n->getItem());
		}

		for (auto e : src->getEdges())
		{
			dst->addEdge(e->getSrc()->getItem(), e->getDst()->getItem(), e->getType());
		}

		return this;
	}

};

#endif // GRAPH_HPP
