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

#include "PtrRelativeRangeAnalysis.h"
#include "recoverFunctionCall.h"

using namespace llvm;
using namespace std;
using namespace lge;

bool PtrRRangeAnalysis::canSelectInfo(Loop *L) {
  if (info.count(L) && (hasFullSideEffectInfo.count(L) == 0))
    return true;
  return false;
}

PtrRRangeAnalysis::ptrData PtrRRangeAnalysis::selectInfo(Loop *L) {
  if (info.count(L))
    return info[L];
  std::map<Value *, PtrLoopInfo> ptr;
  hasFullSideEffectInfo[L] = false;
  return ptr;
}

void PtrRRangeAnalysis::updateInfo(Loop *L, PtrRRangeAnalysis::ptrData ptr) {
  info[L] = ptr;
}

PHINode *PtrRRangeAnalysis::getInductionVariable(Loop *L) {
  //return L->getCanonicalInductionVariable();
  BasicBlock *H = L->getHeader();

  BasicBlock *Incoming = nullptr, *Backedge = nullptr;
  pred_iterator PI = pred_begin(H);
  assert(PI != pred_end(H) && "Loop must have at least one backedge!");
  Backedge = *PI++;
  if (PI == pred_end(H))
    return nullptr; // dead loop
  Incoming = *PI++;
  if (PI != pred_end(H))
    return nullptr; // multiple backedges?

  if (L->contains(Incoming)) {
    if (L->contains(Backedge))
      return nullptr;
    std::swap(Incoming, Backedge);
  } else if (!L->contains(Backedge))
    return nullptr;

  // Loop over all of the PHI nodes, looking for a canonical indvar.
  for (BasicBlock::iterator I = H->begin(); isa<PHINode>(I); ++I) {
    PHINode *PN = cast<PHINode>(I);
    if (ConstantInt *CI =
            dyn_cast<ConstantInt>(PN->getIncomingValueForBlock(Incoming)))
      if (Instruction *Inc =
              dyn_cast<Instruction>(PN->getIncomingValueForBlock(Backedge)))
        if (Inc->getOpcode() == Instruction::Add && Inc->getOperand(0) == PN)
          if (ConstantInt *CI = dyn_cast<ConstantInt>(Inc->getOperand(1)))
            return PN;
  }
  return nullptr;
}

bool PtrRRangeAnalysis::isValidPtr (Value *V) {
  if (isa<Argument>(V) || isa<Instruction>(V) || isa<AllocaInst>(V))
    return true;
  if (GlobalValue *GV = dyn_cast<GlobalValue>(V)) {
    if ((GV->getType()->getTypeID() != Type::PointerTyID) &&
        (GV->getType()->getPointerElementType()->getTypeID() != Type::ArrayTyID)) 
      return false;
  }
  return true;
}

std::pair<Value *, Value *> PtrRRangeAnalysis::findLoopItRange(Loop *L,
                                                               SCEVRangeBuilder *rangeBuilder) {
  PHINode *PHI = rangeBuilder->getInductionVariable(L);
  if (!PHI) {
    hasFullSideEffectInfo[L] = false;
    return std::make_pair(nullptr, nullptr);
  }
  const SCEV *Limit = se->getSCEVAtScope(PHI, L);
  std::vector<const SCEV *> ExprList;
  ExprList.push_back(Limit);
  if (!rangeBuilder->canComputeBoundsFor(Limit)) {
    hasFullSideEffectInfo[L] = false;
    return std::make_pair(nullptr, nullptr);
  }

  Value *low = rangeBuilder->getULowerBound(ExprList);
  Value *up = rangeBuilder->getUUpperBound(ExprList);
  return std::make_pair(low, up);
}

void PtrRRangeAnalysis::findBounds(Loop *L) {
  Module *M = L->getLoopPredecessor()->getParent()->getParent();
  const DataLayout DT = DataLayout(M);
  std::map<Value *, PtrLoopInfo> ptr = selectInfo(L);

  Region *r = rp->getRegionInfo().getRegionFor(L->getLoopPreheader());
  if (!ptrRa->RegionsRangeData[r].HasFullSideEffectInfo)
    r = rp->getRegionInfo().getRegionFor(L->getHeader());
  if (!ptrRa->RegionsRangeData[r].HasFullSideEffectInfo) {
    hasFullSideEffectInfo[L] = false;
    return;
  }

  Instruction *insertPt = L->getHeader()->getTerminator();

  SCEVRangeBuilder rangeBuilder(se, DT, aa, li, dt, r, insertPt);
  InsertPt = insertPt;
  rangeBuilder.setLoop(L);
  for (auto &pair : ptrRa->RegionsRangeData[r].BasePtrsData) {
    //if (RPM.pointerDclInsideLoop(L,pair.first))
    //  continue;
    if (!isValidPtr(pair.first)) {
      hasFullSideEffectInfo[L] = false;
      return;
    }
    rangeBuilder.setPPtr(pair.first);
    rangeBuilder.setRelAnalysisMode(true);
    Value *low = rangeBuilder.getULowerBound(pair.second.AccessFunctions);
    //if (rangeBuilder.isLoopUsed())
    //  low = rangeBuilder.addSymbExp(LParent, low, false);
    Value *up = rangeBuilder.getUUpperBound(pair.second.AccessFunctions);
    up = rangeBuilder.stretchPtrUpperBound(pair.first, up);
    //if (rangeBuilder.isLoopUsed())
    //  up = rangeBuilder.addSymbExp(LParent, up, true);
    ptr[pair.first].constBounds = std::make_pair(low, up);
    rangeBuilder.setRelAnalysisMode(false);
    low = rangeBuilder.getULowerBound(pair.second.AccessFunctions);
    up = rangeBuilder.getUUpperBound(pair.second.AccessFunctions);
    up = rangeBuilder.stretchPtrUpperBound(pair.first, up);
    ptr[pair.first].bounds = std::make_pair(low, up);
    ptr[pair.first].basePtr = pair.first;

    rangeBuilder.setPPtr(rangeBuilder.getInductionVariable(L));
    ptr[pair.first].step =
        std::make_pair(rangeBuilder.findStep(L, pair.first, false),
                       rangeBuilder.findStep(L, pair.first, true));
    ptr[pair.first].itRange = findLoopItRange(L, &rangeBuilder);
//    rangeBuilder.setPPtr(pair.first);

    rangeBuilder.setRelAnalysisMode(true);

    updateInfo(L, ptr);
    calculatePtrRangeToLoop(L, pair.first, &rangeBuilder);
    ptr = selectInfo(L);
  }
}

Value *PtrRRangeAnalysis::getBaseGlobalPtr(Value *V,
                                        SCEVRangeBuilder *rangeBuilder) {
  if (!V) {
    return V;
  }
  if (isa<GlobalValue>(V)) {
    if (Constant *C = dyn_cast<Constant>(V)) {
      if (C->isZeroValue())
        return C;
      LoadInst *LD = new LoadInst(C, C->getName() + "LD", InsertPt); 
      return LD;
    }
    return V;
  }
  if (!isa<Instruction>(V)) {
   return V;
  }
  Instruction *InsertP = cast<Instruction>(V);
  BasicBlock *BB = InsertP->getParent();
  for (auto I = BB->begin(), IE = BB->end(); I != IE; I++) {
    if (I == V) {
      InsertP = ++I;
      break;
    }
  }
  if ((V->getType()->getTypeID() != Type::PointerTyID)) {
    return V;
  }
  APInt AI = APInt(64, 0, true);
  Type *Ty = V->getType()->getPointerElementType();
  Value *Val = Constant::getIntegerValue(Type::getInt64Ty(Ty->getContext()), AI);
  ArrayRef<Value *> IdxList = llvm::ArrayRef<Value*>(const_cast<Value*>(Val));
  if (V->getType()->getTypeID() == Type::PointerTyID) { 
    Type *ElTy = V->getType()->getPointerElementType();
    if ((ElTy->getTypeID() == Type::PointerTyID) && 
       (ElTy->getTypeID() == Type::ArrayTyID) &&
       (ElTy->getTypeID() == Type::VectorTyID)) 
      return GetElementPtrInst::Create(nullptr,V, IdxList,(V->getName() + "PtrGEP"), InsertP);  
  }
  return V;
}

Value *PtrRRangeAnalysis::convertToInt(Value *V,
                                       SCEVRangeBuilder *rangeBuilder) {
  if (!V) {
    return V;
  }  

  V = getBaseGlobalPtr(V, rangeBuilder);
  if (V->getType()->getTypeID() == Type::PointerTyID) {
      return rangeBuilder->InsertCast(Instruction::PtrToInt, V, 
                                     Type::getInt64Ty(V->getContext()));
  }
  if (V->getType()->getTypeID() == Type::ArrayTyID) {
    APInt AI = APInt(64, 0, true);
    Type *Ty = V->getType()->getPointerElementType();
    Value *Val = Constant::getIntegerValue(Type::getInt64Ty(Ty->getContext()), AI);
    return Val;
//    return rangeBuilder->InsertCast(Instruction::PtrToInt, V, 
//                                  V->getType()->getArrayElementType());
  }
  if (V->getType()->getTypeID() == Type::VectorTyID) {
    return rangeBuilder->InsertCast(Instruction::PtrToInt, V, 
                                   V->getType()->getVectorElementType());
  }
  return V;  
}

void PtrRRangeAnalysis::calculatePtrRangeToLoop(Loop *L, Value *Ptr,
                                                SCEVRangeBuilder *rangeBuilder) {
  std::map<Value *, PtrLoopInfo> ptr = selectInfo(L);
  // Finding the Lower and Upper bounds to the Loop's PHI Node.
  Value *PHILower = ptr[Ptr].itRange.first;
  Value *PHI = rangeBuilder->getInductionVariable(L);
  if (!PHI || !ptr[Ptr].step.first) {
    hasFullSideEffectInfo[L] = false;
    return;
  } 
  Type *Ty = Type::getInt64Ty(PHI->getContext());
  Value *Step = ptr[Ptr].step.first;
  Value *CLB = ptr[Ptr].constBounds.first;
  Value *CUB = ptr[Ptr].constBounds.second;

  if (!CLB || !CUB || !Step) {
    hasFullSideEffectInfo[L] = false;
    return;
  }
  if (Step->getType()->getPrimitiveSizeInBits() > Ty->getPrimitiveSizeInBits()) {
    Ty = Step->getType();
  } 
  if (CLB->getType() != Ty) {
    CLB = convertToInt(CLB, rangeBuilder);
    CLB = rangeBuilder->InsertCast(Instruction::SExt, CLB, Ty);
  }
  if (CUB->getType() != Ty) {
    CUB = convertToInt(CUB, rangeBuilder);
    CUB = rangeBuilder->InsertCast(Instruction::SExt, CUB, Ty);
  }
  Value *Window = rangeBuilder->InsertBinop(Instruction::Sub, CUB, CLB);
  ptr[Ptr].window = Window;
  std::vector<const SCEVAddRecExpr *> vct = rangeBuilder->getAllExpr(L, Ptr);
  if (vct.size() == 0) {
    hasFullSideEffectInfo[L] = false;
    return;
  }
  Value *Min = rangeBuilder->getAddRectULowerOrUpperBound(vct, false);
  //Min = rangeBuilder->InsertCast(Instruction::PtrToInt, Min, Step->getType());
  Value *Max = rangeBuilder->getAddRectULowerOrUpperBound(vct, true);
  //Max = rangeBuilder->InsertCast(Instruction::PtrToInt, Max, Step->getType());

  if (!Min || !Max) {
    hasFullSideEffectInfo[L] = false;
    return;
  }

  // Getting the correct size step by step:
  if (PHI->getType() != Ty) {
     PHI = rangeBuilder->InsertCast(Instruction::SExt, PHI, Ty);
  }
  if (PHILower->getType() != Ty) {
    PHILower = rangeBuilder->InsertCast(Instruction::SExt, PHILower, Ty);
  }
  if (Step->getType() != Ty) {
    Step = rangeBuilder->InsertCast(Instruction::SExt, Step, Ty);
  }
  Value *Start = rangeBuilder->InsertBinop(Instruction::Sub, PHI, PHILower);

  // Lower = PHI - PHILower + Min
  Min = convertToInt(Min, rangeBuilder);
  if (Min->getType() != Ty)
    Min = rangeBuilder->InsertCast(Instruction::SExt, Min, Ty);
//  Value *Lower = rangeBuilder->InsertBinop(Instruction::Add, Start, Min);
//  if (Step->getType() != Ty)
//    Lower = rangeBuilder->InsertCast(Instruction::BitCast, Lower, Ty); 
  Value *Lower = rangeBuilder->InsertBinop(Instruction::Mul, Start, Step);
//  Value *Lower = rangeBuilder->InsertBinop(Instruction::Mul, Lower, Step);
  Lower = rangeBuilder->InsertBinop(Instruction::Add, Lower, CLB);
  // Upper = PHI - PHILower + Max
  Max = convertToInt(Max, rangeBuilder);
  if (Max->getType() != Ty)
    Max = rangeBuilder->InsertCast(Instruction::SExt, Max, Ty);
  Value *Upper = rangeBuilder->InsertBinop(Instruction::Add, Start, Max);

  if (Step->getType() != Ty)
    Upper = rangeBuilder->InsertCast(Instruction::BitCast, Upper, Ty);
  Upper = rangeBuilder->InsertBinop(Instruction::Mul, Upper, Step); 
  Upper = rangeBuilder->InsertBinop(Instruction::Add, Upper, CUB);

  // The values that contains the (Lower, Upper) bound to each iteration
  // of the loop. Defined using symbols, i.e. PHI Nodes:
  ptr[Ptr].indVarBounds = std::make_pair(Lower, Upper);
  updateInfo(L, ptr);
}

void PtrRRangeAnalysis::findAllAccessWindows(Function *F) {
  std::map<Loop *, bool> Loops;
  for (auto BB = F->begin(), BE = F->end(); BB != BE; BB++) {
    Loop *L = this->li->getLoopFor(BB);
    if (L && Loops.count(L) == 0) {
      findBounds(L);
      Loops[L] = true;
    }
  }
}

std::pair<Value *, Value *> PtrRRangeAnalysis::getIndVarBounds(Loop *L, Value *Ptr) {
  ptrData ptrD = selectInfo(L);
  return std::make_pair(ptrD[Ptr].indVarBounds.first, ptrD[Ptr].window);
}

bool PtrRRangeAnalysis::runOnFunction(Function &F) {
  this->li = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  this->rp = &getAnalysis<RegionInfoPass>();
  this->aa = &getAnalysis<AliasAnalysis>();
  this->se = &getAnalysis<ScalarEvolution>();
  this->dt = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  this->st = &getAnalysis<ScopeTree>();
  this->ptrRa = &getAnalysis<PtrRangeAnalysis>();

  findAllAccessWindows(&F);

  return true;
}

char PtrRRangeAnalysis::ID = 0;
static RegisterPass<PtrRRangeAnalysis> Z("PtrRRangeAnalysis",
                                         "Recover Ptr Access for each Loop's Iteration.");

//===------------------------ recoverExpressions.cpp ------------------------===//
