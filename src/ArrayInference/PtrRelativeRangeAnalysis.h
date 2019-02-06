//===---------------------- recoverExpressions.h --------------------------===//
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

#include "llvm/Analysis/RegionInfo.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/LoopInfo.h"
#include "PtrRangeAnalysis.h"
#include "SCEVRangeBuilder.h"

#ifndef myutils2
#define myutils2

#include "recoverPointerMD.h"
#endif

using namespace lge;

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
class ArrayInference;
class ScopeTree;

class PtrRRangeAnalysis : public FunctionPass {

  private:

  //===---------------------------------------------------------------------===
  //                              Data Structs
  //===---------------------------------------------------------------------===
  typedef struct PtrLoopInfo {
    Value *basePtr;
    std::pair<Value*, Value*> bounds;
    std::pair<Value*, Value*> itRange; 
    std::pair<Value*, Value*> step;
    std::pair<Value*, Value*> constBounds;
    std::pair<Value*, Value*> indVarBounds;
    Value *window;
  } PtrLoopInfo;
  
  typedef std::map<Value*, PtrLoopInfo> ptrData;

  std::map<Loop*, ptrData> info;
  //===---------------------------------------------------------------------===

  void calculatePtrRangeToLoop(Loop *L, Value *Ptr, SCEVRangeBuilder
                                                    *rangeBuilder);

  bool isValidPtr (Value *V);

  Instruction *InsertPt;

  ptrData selectInfo (Loop *L);

  void updateInfo (Loop *L, ptrData ptr);

  PHINode *getInductionVariable(Loop *L);
  
  void findBounds (Loop *L);

  Value *getBaseGlobalPtr(Value *V, SCEVRangeBuilder *rangeBuilder);

  Value *convertToInt(Value *V, SCEVRangeBuilder *rangeBuilder);

  std::pair<Value*, Value*> findLoopItRange (Loop *L,
                                             SCEVRangeBuilder *rangeBuilder);

  void findAllAccessWindows (Function *F);

public:

  //===---------------------------------------------------------------------===
  //                              Data Structs
  //===---------------------------------------------------------------------===
   std::map<Loop*, bool> hasFullSideEffectInfo; 
  //===---------------------------------------------------------------------===

  static char ID;

  PtrRRangeAnalysis() : FunctionPass(ID) {};
 
  // We need to insert the Instructions for each source file.
  virtual bool runOnFunction(Function &F) override;

  bool canSelectInfo(Loop *L);

  std::pair<Value*, Value*> getIndVarBounds(Loop *L, Value *Ptr);

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<RegionInfoPass>();
      AU.addRequired<AliasAnalysis>();
      AU.addRequired<ScalarEvolution>();
      AU.addRequiredTransitive<LoopInfoWrapperPass>();
      AU.addRequired<DominatorTreeWrapperPass>();
      AU.addRequired<ScopeTree>();
      AU.addRequired<PtrRangeAnalysis>();
      AU.setPreservesAll();
  }

  RegionInfoPass *rp;
  AliasAnalysis *aa;
  ScalarEvolution *se;
  LoopInfo *li;
  DominatorTree *dt;
  ScopeTree *st;
  PtrRangeAnalysis *ptrRa;
};

}

//===---------------------- recoverExpressions.h --------------------------===//
