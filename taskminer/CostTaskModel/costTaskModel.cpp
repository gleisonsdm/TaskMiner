//===------------------------- costTaskModel.cpp --------------------------===//
//
//                     The LLVM Compiler Infrastructure
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

#include "llvm/IR/Module.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"

#include "costTaskModel.h" 

using namespace llvm;
using namespace std;

void CostTaskModel::collectRegionInfo(Function *F, Region *R, dataFunction *df) {
  dataRegion dr;
  dr.n_inst = 0;
  dr.n_inst_mem = 0;
  dr.n_calls = 0;
  for (auto BB = R->block_begin(), BE = R->block_end() ; BB != BE; BB++) {
    for (auto II = BB->begin(), IE = BB->end(); II != IE; II++) {
      dr.n_inst++;

      if (isa<StoreInst>(II) || isa<LoadInst>(II) ||
          isa<GetElementPtrInst>(II))
        dr.n_inst_mem++;

      if (CallInst *CI = dyn_cast<CallInst>(II)) {
        Function *Fn = CI->getCalledFunction();
        if (!Fn->isDeclaration() && !Fn->isIntrinsic() &&
            !Fn->hasAvailableExternallyLinkage()) {
          dr.n_calls++;
          if (Fn == F)
            dr.isRecursive = true;
        }
      }

    }
  }
  df->regions[R] = dr;
  for (auto SR = R->begin(), SRE = R->end(); SR != SRE; ++SR)
    collectRegionInfo(F, &(**SR), df);
}

void CostTaskModel::collectFunctionInfo(Function *F) {
  dataFunction df;
  df.n_inst = 0;
  df.n_inst_mem = 0;
  df.n_calls = 0;
  df.isRecursive = false;
  for (auto BB = F->begin(), BE = F->end() ; BB != BE; BB++) {
    for (auto II = BB->begin(), IE = BB->end(); II != IE; II++) {
      df.n_inst++;

      if (isa<StoreInst>(II) || isa<LoadInst>(II) ||
          isa<GetElementPtrInst>(II))
        df.n_inst_mem++;

      if (CallInst *CI = dyn_cast<CallInst>(II)) {
        Function *Fn = CI->getCalledFunction();
        if (!Fn->isDeclaration() && !Fn->isIntrinsic() &&
            !Fn->hasAvailableExternallyLinkage()) {
          df.n_calls++;
          if (Fn == F)
            df.isRecursive = true;
        }
      }

    }
  }
  Region *region = rp->getRegionInfo().getRegionFor(F->begin());
  Region *topRegion = region;
  while (region != NULL) {
    topRegion = region;
    region = region->getParent();
  }
  collectRegionInfo(F, topRegion, &df);
  scFunctions[F] = df;
}

bool CostTaskModel::runOnModule (Module &M) {
  this->rp = &getAnalysis<RegionInfoPass>();

  for (Module::iterator F = M.begin(), FE = M.end(); F != FE; ++F) { 
    collectFunctionInfo(&(*F));
  }
}  

char CostTaskModel::ID = 0;

static RegisterPass<CostTaskModel> Z("costTaskModel",
"Define cost for each function as a task.");


//===-------------------------- costTaskModel.cpp -------------------------===//

