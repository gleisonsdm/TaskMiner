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
    std::vector<Loop*> loops;
    errs() << "0\n";
    if (LI->begin() == LI->end())
      return PreOrderLoops;

    errs() << "1\n"; 
//    for (auto loopIt = LI->begin(); loopIt != LI->end(); loopIt++)
//      loops.push_back(*loopIt);
		for (auto loopIt = LI->begin(); loopIt != LI->end(); loopIt++)
//    for (int i = loops.size(), ie = 0; i != ie; i--)
		{
       if (!PreOrderWorklist.empty())
         return PreOrderLoops;
       errs() << "2\n";
//       PreOrderWorklist.push_back(loops[i]);
		 PreOrderWorklist.push_back(*loopIt);
		 do {
		   Loop* L = PreOrderWorklist.pop_back_val();
		   // Sub-loops are stored in forward program order, but will process the
		   // worklist backwards so append them in reverse order.
       errs() << "3\n";
		   PreOrderWorklist.append(L->rbegin(), L->rend());
		   PreOrderLoops.push_back(L);
		 } while (!PreOrderWorklist.empty());
		}

		return PreOrderLoops;
	}

};

