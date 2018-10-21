//===--------------------------- TestRFCall.cpp ---------------------------===//
//
// This file is distributed under the Universidade Federal de Minas Gerais - 
// UFMG Open Source License. See LICENSE.TXT for details.
//
// Copyright (C) 2015   Gleison Souza Diniz Mendon?a
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#include <fstream>
#include <queue>

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

#include "PtrRangeAnalysis.h"

#include "testRFCall.h"
#include "recoverFunctionCall.h"

using namespace llvm;
using namespace std;
using namespace lge;

bool TestRFCall::runOnFunction(Function &F) {
  this->li = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  this->rp = &getAnalysis<RegionInfoPass>();
  this->aa = &getAnalysis<AliasAnalysis>();
  this->se = &getAnalysis<ScalarEvolution>();
  this->dt = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  this->ptrRA = &getAnalysis<PtrRangeAnalysis>();
  this->rn = &getAnalysis<RecoverNames>();
  this->rr = &getAnalysis<RegionReconstructor>();
  this->st = &getAnalysis<ScopeTree>();
  
  RecoverFunctionCall rfc;
  int comp = 1;

  for (auto BB = F.begin(), BE = F.end(); BB != BE; BB++)
    for (auto I = BB->begin(), IE = BB->end(); I != IE; I++) {
      if (CallInst *CI = dyn_cast<CallInst>(I)) {
        std::string computationName = "TM" + std::to_string(comp);
        comp++;
        rfc.setNAME(computationName);
        rfc.setRecoverNames(rn);
        rfc.initializeNewVars();
        rfc.annotataFunctionCall(CI, ptrRA, rp, aa, se, li, dt);
      }
    }
  return true;
}

char TestRFCall::ID = 0;
static RegisterPass<TestRFCall> Z("testRFCall",
"Testing Recover Function Call to source code.");

//===------------------------ writeExpressions.cpp ------------------------===//
