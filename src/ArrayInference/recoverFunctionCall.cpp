//===-------------------------- recoverCode.cpp ---------------------------===//
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
//
//===----------------------------------------------------------------------===//

#include <fstream>
#include <queue>

#include "llvm/IR/DIBuilder.h" 
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/MC/MCExpr.h"
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

#include "recoverFunctionCall.h" 
#include "PtrRangeAnalysis.h"
#include "restrictifier.h"

using namespace llvm;
using namespace std;
using namespace lge;

#define ERROR_VALUE -1

int RecoverFunctionCall::getLineNo (Value *V) {
  if (!V)
    return ERROR_VALUE;
  if (Instruction *I = dyn_cast<Instruction>(V))
    if (I)
      if (MDNode *N = I->getMetadata("dbg"))
        if (N)
          if (DILocation *DI = dyn_cast<DILocation>(N))
            return DI->getLine();
  return ERROR_VALUE;
}

void RecoverFunctionCall::getSimbolsContext (CallInst *CI,
                                std::map<std::string, std::string> & sbls) {

  Module *M = CI->getParent()->getParent()->getParent();
  const DataLayout DT = DataLayout(M);
  Function *F = CI->getCalledFunction();
  auto argI = F->arg_begin(), argE = F->arg_end();

  for (unsigned int i = 0, ie = CI->getNumArgOperands(); i != ie; i++, argI++) {
    Value *v = CI->getArgOperand(i);
    int var1 = -1, var2 = -1;
    
    std::string oriName = getAccessString(v, std::string(), &var1, &DT);
    if (var1 != -1)
      oriName = (NAME + "[" + std::to_string(var1) + "]");
    
    std::string cntName = getAccessString(argI, std::string(), &var2, &DT);
    if (var2 != -1)
      cntName = (NAME + "[" + std::to_string(var2) + "]");

    sbls[cntName] = oriName;
  }
}

void RecoverFunctionCall::findAllKnowedSimbols() {
  unsigned int size = computationRef.size();
  std::string buffer = std::string();
  for (unsigned int i = 0; i < size; i++) {
    if (computationRef[i] != ' ') {
      buffer += computationRef[i];
    }
    bool check = true;
    if (buffer.find("long") != std::string::npos ||
        buffer.find("int") != std::string::npos ||
        buffer.find("AI") != std::string::npos ||
        buffer.find("+") != std::string::npos ||
        (buffer.find("-") != std::string::npos && buffer.size() == 1 ) ||
        buffer.find("*") != std::string::npos ||
        buffer.find("/") != std::string::npos ||
        buffer.find("?") != std::string::npos ||
        buffer.find(">") != std::string::npos ||
        buffer.find("<") != std::string::npos ||
        buffer.find("=") != std::string::npos ||
        buffer.find("|") != std::string::npos ||
        buffer.find("pragma") != std::string::npos ||
        buffer.find("acc") != std::string::npos ||
        buffer.find("data") != std::string::npos ||
        buffer.find("kernels") != std::string::npos)
       check = false;
       
    if (!check || buffer.size() < 1)
      continue;
    if (buffer[0] == '(')
      buffer.erase(buffer.begin(), (buffer.begin() + 1));
    if (buffer[(buffer.size()-1)] == ')' || buffer[(buffer.size()-1)] == ';')
      buffer.erase(buffer.begin() + (buffer.size() - 1), buffer.end());

    long long int num = 0;
    if (TryConvertToInteger(buffer, &num))
      continue;
    
    simbols[buffer] = false;


    buffer = std::string();
  }

}

void RecoverFunctionCall::getSubLoopsLimits (
                                std::map<Loop*, DataLimits>  loopsLimits) {
  // HERE
}

void RecoverFunctionCall::collectLoopInfo (Loop *L, Loop *Parent, CallInst *CI,
                                        PtrRangeAnalysis *ptrRA, 
                                        RegionInfoPass *rp, AliasAnalysis *aa,
                                        ScalarEvolution *se, LoopInfo *li,
                                        DominatorTree *dt,
                                   std::map<Loop*, DataLimits> & loopsLimits) {
  if (!L || loopsLimits.count(L) != 0)
    return;
  Instruction *I = L->getCanonicalInductionVariable();
  const SCEV* Limit = se->getSCEVAtScope(I, L);

  for (Loop *SubLoop : L->getSubLoops())
    collectLoopInfo(SubLoop, L, CI, ptrRA, rp, aa, se, li, dt, loopsLimits);

  Module *M = L->getLoopPredecessor()->getParent()->getParent();;
  const DataLayout DT = DataLayout(M);
  Region *r = rp->getRegionInfo().getRegionFor(L->getHeader());
  Instruction *insertPt = L->getHeader()->getTerminator();
  std::vector<const SCEV *> ExprList;
  ExprList.push_back(Limit);
  SCEVRangeBuilder rangeBuilder(se, DT, aa, li, dt, r, insertPt);
  if (!rangeBuilder.canComputeBoundsFor(Limit))
    return;

  Value *low = rangeBuilder.getULowerBound(ExprList);
  Value *up = rangeBuilder.getUUpperBound(ExprList);
  int var1 = -1, var2 = -1;
  std::string lLimit = getAccessString(low, std::string(), &var1, &DT);
  std::string uLimit = getAccessString(up, std::string(), &var2, &DT);
  loopsLimits[L].lowerBound = lLimit;
  if (var1 != -1)
    loopsLimits[L].lowerBound = (NAME + "[" + std::to_string(var1) + "]");
  loopsLimits[L].upperBound = uLimit;
  if (var2 != -1)
    loopsLimits[L].upperBound = (NAME + "[" + std::to_string(var2) + "]");
  loopsLimits[L].Parent = Parent;
  loopsLimits[L].hasParent = Parent;
  errs() << "Limit = " << lLimit << " <> " << uLimit << "\n";
}

void RecoverFunctionCall::annotataFunctionCall (CallInst *CI,
                                        PtrRangeAnalysis *ptrRA, 
                                        RegionInfoPass *rp, AliasAnalysis *aa,
                                        ScalarEvolution *se, LoopInfo *li,
                                        DominatorTree *dt) {
  Function *F = CI->getCalledFunction();
  if (F->isDeclaration() || F->isIntrinsic())
    return;
  std::map<Loop*, DataLimits> loopsLimits;
  for (auto BB = F->begin(), BE = F->end(); BB != BE; BB++) {
    Loop *L = li->getLoopFor(BB);
    collectLoopInfo(L, nullptr, CI, ptrRA, rp, aa, se, li, dt, loopsLimits);
  } 
}
//===---------------------------- RecoverCode.cpp -------------------------===//
