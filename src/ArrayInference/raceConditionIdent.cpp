//===---------------------- recoverExpressions.cpp ------------------------===//
//
// This file is distributed under the Universidade Federal de Minas Gerais - 
// UFMG Open Source License. See LICENSE.TXT for details.
//
// Copyright (C) 2015   Gleison Souza Diniz Mendon?a
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/RegionInfo.h"  
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/DIBuilder.h" 
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/ADT/Statistic.h"
#include "raceConditionIdent.h"

using namespace llvm;
using namespace std;

PHINode *ParallelRaceCondition::getInductionVariable(Loop *L, ScalarEvolution *SE) {
   PHINode *InnerIndexVar = L->getCanonicalInductionVariable();
   if (InnerIndexVar)
     return InnerIndexVar;
   if (L->getLoopLatch() == nullptr || L->getLoopPredecessor() == nullptr)
     return nullptr;
   for (BasicBlock::iterator I = L->getHeader()->begin(); isa<PHINode>(I); ++I) {
     PHINode *PhiVar = cast<PHINode>(I);
     Type *PhiTy = PhiVar->getType();
     if (!PhiTy->isIntegerTy() && !PhiTy->isFloatingPointTy() &&
        !PhiTy->isPointerTy())
       return nullptr;
     const SCEVAddRecExpr *AddRec =
         dyn_cast<SCEVAddRecExpr>(SE->getSCEV(PhiVar));
     if (!AddRec || !AddRec->isAffine())
       continue;
     const SCEV *Step = AddRec->getStepRecurrence(*SE);
     if (!isa<SCEVConstant>(Step))
       continue;
     // Found the induction variable.
     // FIXME: Handle loops with more than one induction variable. Note that,
     // currently, legality makes sure we have only one induction variable.
     return PhiVar;
   }
   return nullptr;
}

bool ParallelRaceCondition::safe(Function *F, Instruction *I, std::map<Instruction*, bool> & valid) {
  if (valid.count(I) != 0)
    return valid[I];
  for (int i = 0; i < I->getNumOperands(); i++) {
    for (auto It = indVars[F].begin(), ItE = indVars[F].end(); It != ItE; It++) {
      if (I->getOperand(i) == It->second)
        return false;
      if (Instruction *I = dyn_cast<Instruction>(I->getOperand(i)))
        if (!safe(F, I, valid))
      return false;
    }
  }
  return true;
}

void ParallelRaceCondition::analyzeFunction(Function *F) {
  for (auto B = F->begin(), BE = F->end(); B != BE; B++) {
    Loop *L = this->li->getLoopFor(B);
    if (!L)
      continue;
    Instruction *indVar = getInductionVariable(L, this->se);
    if (indVars.count(F) == 0) {
      std::map<Loop*, Instruction*> mp;
      indVars[F] = mp;
    }
    if (L && (indVars[F].count(L) == 0))
      indVars[F][L] = indVar;
    std::map<Instruction*, bool> valid;
    for (auto I = B->begin(), IE = B->end(); I != IE; I++) {
      if (!safe(F, I, valid))
        raceConditions[F][L].push_back(I);
    }
  }
}

bool ParallelRaceCondition::runOnFunction(Function &F) {
  this->li = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  this->rp = &getAnalysis<RegionInfoPass>();
  this->aa = &getAnalysis<AliasAnalysis>();
  this->se = &getAnalysis<ScalarEvolution>();
  this->dt = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();

  if (!F.isDeclaration() && !F.isIntrinsic() &&
      !F.hasAvailableExternallyLinkage()) {
    analyzeFunction(&F);
  }
  return true;
}

char ParallelRaceCondition::ID = 0;
static RegisterPass<ParallelRaceCondition> Z("ParallelRaceCondition",
"Find race conditions on loops.");

//===------------------------ recoverExpressions.cpp ------------------------===//
