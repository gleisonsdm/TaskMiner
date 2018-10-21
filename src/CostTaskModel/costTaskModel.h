//===-------------------------- costTaskModel.h ---------------------------===//
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

namespace llvm {
class DominatorTree;

class DominanceFrontier;
struct PostDominatorTree;
class Value;
class Region;
class Instruction;
class LoopInfo;

class CostTaskModel : public ModulePass {

  private:

  typedef struct dataR {
    Function *func;
    int n_inst;
    int n_inst_mem;
    int n_calls;
    bool isRecursive;
  } dataRegion;

  typedef struct dataF {
    int n_inst;
    int n_inst_mem;
    int n_calls;
    bool isRecursive;
    std::map<Region*, dataRegion> regions;
  } dataFunction;

  //===---------------------------------------------------------------------===
  //                              Data Structs
  //===---------------------------------------------------------------------===
  std::map<Function*, dataFunction> scFunctions;
  //===---------------------------------------------------------------------===

  public:

  static char ID;

  CostTaskModel() : ModulePass(ID) {};

  // Colect the cost to the function F.
  void collectFunctionInfo(Function *F);

  // Colect the cost to the region R.
  void collectRegionInfo(Function *F, Region *R, dataFunction *df);

  // Analyze the moule and provide the static data for each function..
  virtual bool runOnModule(Module &M);

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<RegionInfoPass>();
    AU.setPreservesAll();
  }
  
  RegionInfoPass *rp;
  
};

}

//===-------------------------- costTaskModel.h ---------------------------===//
