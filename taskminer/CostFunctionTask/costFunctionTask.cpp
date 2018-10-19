//===----------------------- costFunctionTask.cpp -------------------------===//
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

#include "costFunctionTask.h" 

using namespace llvm;
using namespace std;
using namespace lge;

void CostFunctinTask::collectFunctionInfo(Function *F) {
  dataFunction df;
  df.n_inst = 0;
  df.n_inst_mem = 0;
  df.n_calls = 0;
  for (auto BB = F->begin(), BE = F->end() ; BB != BE; BB++) {
    for (auto II = BB->block_begin(), IE = BB->block_end(); II != IE; II++) {
      df.n_inst++;
      if (isa<StoreInst>(II) || isa<LoadInst>(II) ||
          isa<GetElementPtrInst>(II))
        df.n_inst_mem++;
      // Ignore debug Call Inst:
      if (isa<CallInst>(II))
        df.n_calls++;
    }
  }
  // Find the other struct members:
  //
  //
  scFunctions[F] = df;
}

bool CostFunctionTask::runOnModule (Module &M) {
  for (Module::iterator F = M.begin(), FE = M.end(); F != FE; ++F) { 
    collectFunctionInfo(&(*F));
  }
}  

char ::ID = 0;

static RegisterPass<CostFunctionTask> Z("costFunctionTask",
"Define cost for each function as a task.");


//===-------------------------- writeInFile.cpp --------------------------===//

