//===------------------------ costFunctionTask.h --------------------------===//
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

#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/Analysis/RegionInfo.h"
#include "llvm/Analysis/LoopInfo.h"

using namespace lge;

namespace llvm {
class DominatorTree;

class DominanceFrontier;
struct PostDominatorTree;
class Value;
class Region;
class Instruction;
class LoopInfo;
class ArrayInference;

class CostFunctionTask : public ModulePass {

  private:

  struct dataF {
    int n_inst;
    int n_inst_mem;
    int n_loops;
    int n_nest_loops;
    int n_calls;
    bool isRecursive;
  } dataFunction;

  //===---------------------------------------------------------------------===
  //                              Data Structs
  //===---------------------------------------------------------------------===
  std::map<Function*, dataFunction> scFunctions;
  //===---------------------------------------------------------------------===

  public:

  static char ID;

  CostFunctionTask() : ModulePass(ID) {};

  // Calculates and provides a cost to the function F.
  int getFunctionCostH1 (Function *F);

  // Colect the cost to the function F.
  void collectFunctionInfo(Function *F);

  // Analyze the moule and provide the static data for each function..
  virtual bool runOnModule(Module &M);

  /*virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<WriteExpressions>();
    AU.addRequired<RecoverExpressions>();
    AU.setPreservesAll();
  }*/
  
  // Pass that will insert comments in source file.
  //WriteExpressions *we;

  // Insert tasks into the source file.
  //RecoverExpressions *re;
  
};

}

//===------------------------ writeInFIle.h --------------------------===//
