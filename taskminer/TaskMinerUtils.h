//LLVM IMPORTS
#include "llvm/Analysis/LoopInfo.h"

//STL IMPORTS
#include <vector>
#include <algorithm>

class TaskMinerUtils
{

public:
	std::vector<Loop*> getLoopsInPreorder(LoopInfo* LI)
	{
		std::vector<Loop*> PreOrderLoops;
		SmallVector<Loop*, 4> PreOrderWorklist;
		for (auto loopIt = LI->begin(); loopIt != LI->end(); loopIt++)
		{
		 PreOrderWorklist.push_back(*loopIt);
		 do {
		   Loop* L = PreOrderWorklist.pop_back_val();
		   // Sub-loops are stored in forward program order, but will process the
		   // worklist backwards so append them in reverse order.
		   PreOrderWorklist.append(L->rbegin(), L->rend());
		   PreOrderLoops.push_back(L);
		 } while (!PreOrderWorklist.empty());
		}

		return PreOrderLoops;
	}

};

