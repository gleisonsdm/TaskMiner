//===--------------------- ptrAccessTypeAnalysis.h ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the Universidade Federal de Minas Gerais -
// UFMG Open Source License. See LICENSE.TXT for details.
//
// Copyright (C) 2019   Gleison Souza Diniz Mendon?a
//
//===----------------------------------------------------------------------===//
// 
//===----------------------------------------------------------------------===//

#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/Analysis/RegionInfo.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/LoopInfo.h"
#include <map>
#include <vector>
#include <queue>

namespace llvm {
class ScalarEvolution;
class AliasAnalysis;
class SCEV;
class DominatorTree;

class DominanceFrontier;
struct PostDominatorTree;
class Value;
class Region;
class Instruction;
class LoopInfo;

#define UNKNOUM 0
#define READ 1
#define WRITE 2
#define READWRITE 3

class PtrAccessTypeAnalysis : public ModulePass {

  private:

  //===---------------------------------------------------------------------===
  //                              Data Structs
  //===---------------------------------------------------------------------===
  typedef struct datamap {
    // The pointer in the current function:
    Value *currentValue;
    // The pointer in the context of target CallInst:
    Value *targetValue;
  } datamap;

  std::map<Function*, std::map<CallInst*, std::vector<datamap> > > ptrDeps;

  std::map<Function*, bool> isFullAnalyzed;

  std::map<Function*, std::vector<CallInst*> > dependences;

  std::map<Function*, std::vector<Function*> > parentDependences;

  std::map<Function*, bool> isValidFunction;
  //===---------------------------------------------------------------------===

  void findDependences(Module *M);

  bool validateGetElementPtrInstOps(Value *V);

  bool validateGetElementPtrInst(Value *V);

  void analyzeFunction(Function *F);

  bool findAllDependences(Function *F, Function *FTop,
                          std::map<Function*, bool> & knowed);

  void findMapToArguments(Function *F);

  public:

  //===---------------------------------------------------------------------===
  //                              Data Structs
  //===---------------------------------------------------------------------===
 
  std::map<Function*, std::map<Value*, char> > accessType;

  //===---------------------------------------------------------------------===

  static char ID;

  PtrAccessTypeAnalysis() : ModulePass(ID) {};

  Value *getBasePtr(Value *V);

  void getListFor(Function *F, std::map<Value*, char> & accessTy) {
    if (accessType.count(F) > 0)
      for (std::map<Value*, char>::iterator I = accessType[F].begin(),
           IE = accessType[F].end(); I != IE; I++)
        accessTy[I->first] = I->second;
  }
  
  // We need Insert the Instructions for each source file.
  virtual bool runOnModule(Module &M);

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
  }
  
};

}

//===-------------------- ptrAccessTypeAnalysis.h -------------------------===//
