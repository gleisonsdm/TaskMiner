//===-------------------- ptrAccessTypeAnalysis.cpp -----------------------===//
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
#include "llvm/IR/Module.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"

#include "ptrAccessTypeAnalysis.h"

using namespace llvm;

Value *PtrAccessTypeAnalysis::getBasePtr(Value *V) {
  if (LoadInst *LD = dyn_cast<LoadInst>(V))
    return getBasePtr(LD->getPointerOperand());
  else if (StoreInst *ST = dyn_cast<StoreInst>(V))
    return getBasePtr(ST->getPointerOperand());
  else if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(V))
    return getBasePtr(GEP->getPointerOperand());
  else if (CallInst *CI = dyn_cast<CallInst>(V)) 
    return CI;
  else if (AllocaInst *AI = dyn_cast<AllocaInst>(V))
    return AI;
  else if (GlobalValue *GV = dyn_cast<GlobalValue>(V))
    return GV;
  else if (Argument *Arg = dyn_cast<Argument>(V))
    return Arg;
  else if (Instruction *I = dyn_cast<Instruction>(V))
    return getBasePtr(I->getOperand(0));
  return V;
}

bool PtrAccessTypeAnalysis::validateGetElementPtrInstOps(Value *V) {
  if (LoadInst *LD = dyn_cast<LoadInst>(V)) 
    return false;

  // Try to find in each base pointer:
  if (Instruction *I = dyn_cast<Instruction>(V)) {
    if (isa<PHINode>(I))
      return validateGetElementPtrInstOps(I->getOperand(0));
    for (int i = 0, ie = I->getNumOperands(); i != ie; i++) {
        return validateGetElementPtrInstOps(I->getOperand(i));
    }
  }
  return true;
}

bool PtrAccessTypeAnalysis::validateGetElementPtrInst(Value *V) {
  // Looking to the offset of a memory instruction, it is not
  // valid to use a pointer that depends of another pointer. I.E:
  //
  //     Vec[ Vect[ i ] ]
  //
  // It's impossble to inffer any correct information about the pointer.
  if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(V)) {
    for (int i = 1, ie = GEP->getNumOperands(); i != ie; i++)
       if (validateGetElementPtrInstOps(GEP->getOperand(i)) == false)
         return false;
  }
  return true;
}

bool PtrAccessTypeAnalysis::findAllDependences(Function *F, Function *FTop, 
                                        std::map<Function*, bool> & knowed) {
  bool returnVal = false;
  for (auto BB = F->begin(), BE = F->end(); BB != BE; BB++) {
    for (auto II = BB->begin(), IE = BB->end(); II != IE; II++) {
      if (CallInst *CI = dyn_cast<CallInst>(II)) {
        Function *FF = CI->getCalledFunction();
        if ((FF == FTop) && (F != FF))
          returnVal = true;
        if ((knowed.count(FF) == 0) && (isFullAnalyzed[FF] == false)) {
          knowed[FF] = true;
          returnVal = returnVal | findAllDependences(FF, FTop, knowed);
        }
      }
    }
  }
  return returnVal;
}

void PtrAccessTypeAnalysis::analyzeFunction(Function *F) {
  isFullAnalyzed[F] = true;
  for (auto BB = F->begin(), BE = F->end(); BB != BE; BB++) {
    for (auto II = BB->begin(), IE = BB->end(); II != IE; II++) {
      if (CallInst *CI = dyn_cast<CallInst>(II)) {
        Function *FF = CI->getCalledFunction();
        if (FF == nullptr)
          continue;
        if (FF->isDeclaration() || FF->isIntrinsic() ||
            FF->hasAvailableExternallyLinkage())
//          if (FF->isIntrinsic() || FF->hasAvailableExternallyLinkage())
          continue;
        std::vector<CallInst*> tmpVec;
        std::vector<Function*> tmpVec2;
        if ((F != FF) && (dependences.count(F) == 0)) {
          dependences[F] = tmpVec;
          parentDependences[FF] = tmpVec2; 
        }
        if (F != FF) {
          dependences[F].push_back(CI);
          parentDependences[FF].push_back(F);
        }
        isFullAnalyzed[F] = false;
      } 
      else if (LoadInst *LD = dyn_cast<LoadInst>(II)) {
        Value *basePtr = getBasePtr(LD->getPointerOperand());
        if (accessType[F].count(basePtr) == 0)
          accessType[F][basePtr] = UNKNOUM;
        accessType[F][basePtr] = accessType[F][basePtr] | READ;
      }
      else if (StoreInst *ST = dyn_cast<StoreInst>(II)) {
        Value *basePtr = getBasePtr(ST->getPointerOperand());
        if (accessType[F].count(basePtr) == 0)
          accessType[F][basePtr] = UNKNOUM;
        accessType[F][basePtr] = accessType[F][basePtr] | WRITE;
      }
      else if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(II)) {
        if (isValidFunction.count(F) == 0)
          isValidFunction[F] = validateGetElementPtrInst(GEP);
        else
          isValidFunction[F] = (isValidFunction[F] &
                                validateGetElementPtrInst(GEP));
      }
    }
  }  
}

void PtrAccessTypeAnalysis::findMapToArguments(Function *F) {
  for (auto BB = F->begin(), BE = F->end(); BB != BE; BB++) {
    for (auto II = BB->begin(), IE = BB->end(); II != IE; II++) {
      if (CallInst *CI = dyn_cast<CallInst>(II)) {
        Function *FF = CI->getCalledFunction();
        if (FF == nullptr)
          continue;
        if (FF->isDeclaration() || FF->isIntrinsic() ||
            FF->hasAvailableExternallyLinkage())
          continue;
        datamap dtm;
//        errs() << "=================================================\n";
//        CI->dump();
        unsigned int i = 0;
        std::vector<datamap> tmpVect;
        if (FF->arg_begin() != FF->arg_end())
          ptrDeps[F][CI] = tmpVect;
        for (auto AI = FF->arg_begin(), AE = FF->arg_end(); AI != AE; AI++, i++) {
          dtm.currentValue = CI->getArgOperand(i);
          dtm.targetValue = AI;
//          errs() << "===-------------------------------------------===\n";
//          dtm.currentValue->dump();
//          dtm.targetValue->dump();
//          errs() << "===-------------------------------------------===\n";
          ptrDeps[F][CI].push_back(dtm);
        }
      }
    }
  }
}

void PtrAccessTypeAnalysis::findDependences(Module *M) {
  std::queue<Function*> funcWorkList;
  for (Module::iterator FI = M->begin(), FE = M->end(); FI != FE; FI++) {
    if (FI->isDeclaration() || FI->isIntrinsic() ||
        FI->hasAvailableExternallyLinkage())
      continue;
 
        funcWorkList.push(FI);
  }
  while (!funcWorkList.empty()) {
    Function *F = funcWorkList.front();
    funcWorkList.pop();
    if (ptrDeps.count(F) > 0) {
      for (std::map<CallInst*, std::vector<datamap> >::iterator I =
        ptrDeps[F].begin(), IE = ptrDeps[F].end(); I != IE; I++) {
        CallInst *CI = I->first;  
        Function *FF = CI->getCalledFunction();
        bool updated = false;
        for (int j = 0, je = I->second.size(); j != je; j++) {
          if (I->second[j].currentValue->getType()->isPointerTy() &&
              I->second[j].targetValue->getType()->isPointerTy()) {
            Value *basePtrCurrent = getBasePtr(I->second[j].currentValue);
            Value *basePtrTarget = getBasePtr(I->second[j].targetValue); 
            char oldStats = accessType[F][basePtrCurrent];
            accessType[F][basePtrCurrent] = accessType[F][basePtrCurrent] |
                                            accessType[FF][basePtrTarget];
            if (oldStats != accessType[F][basePtrCurrent])
               updated = true;
          }
        }
        if (updated) {
          // Insert the functions that depends of F:
          if (parentDependences.count(F) != 0) {
            for (int i = 0, ie = parentDependences[F].size(); i != ie; i++) {
              funcWorkList.push(parentDependences[F][i]);
            }
          }
        }
      }
    } 
  }  
}

bool PtrAccessTypeAnalysis::runOnModule (Module &M) {

  for (Module::iterator FI = M.begin(), FE = M.end(); FI != FE; FI++) {
    if (FI->isDeclaration() || FI->isIntrinsic() ||
        FI->hasAvailableExternallyLinkage())
      continue;
    analyzeFunction(FI);
    findMapToArguments(FI);
  }
  findDependences(&M);
/*  for (Module::iterator FI = M.begin(), FE = M.end(); FI != FE; FI++) {
    
    if (accessType[FI].begin() != accessType[FI].end())
    errs() << "\n" << FI->getName() << "\n====================================\n";
    for (std::map<Value*, char>::iterator ATI = accessType[FI].begin(),
         ATE = accessType[FI].end(); ATI != ATE; ATI++) {
       char result = (ATI->second + '0');
       ATI->first->dump();
       errs() << result;
       errs() << "\n---------------------------------\n";
    }
  }*/
 
  return false;
}

char PtrAccessTypeAnalysis::ID = 0;
static RegisterPass<PtrAccessTypeAnalysis> X("ptr-access",
"Mapping memory access type.");


//===-------------------------- ptrAccessTypeAnalysis.cpp --------------------------===//

