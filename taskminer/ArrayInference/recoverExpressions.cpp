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

#include "recoverExpressions.h"
#include "recoverFunctionCall.h"

using namespace llvm;
using namespace std;
using namespace lge;

#define DEBUG_TYPE "recoverExpressions"
#define ERROR_VALUE -1

STATISTIC(numRA , "Number of regions annotated"); 

void RecoverExpressions::setTasksList(std::list<Task*> taskList) {
  this->tasksList = taskList;
} 

void RecoverExpressions::setTasksCalls(std::set<CallInst*> taskCalls) {
  this->tasksCalls = taskCalls;
}

int RecoverExpressions::getIndex() {
  return this->index;
}

int RecoverExpressions::getNewIndex() {
  return (++(this->index));
}

void RecoverExpressions::addCommentToBLine (std::string Comment,
                                         unsigned int Line) {     
  if (Comments.count(Line) == 0)
    Comments[Line] = Comment;
  else if (Comments[Line].find(Comment) == std::string::npos)
    Comments[Line] = Comment + Comments[Line];
} 

void RecoverExpressions::addCommentToLine (std::string Comment,
                                         unsigned int Line) {     
  if (Comments.count(Line) == 0)
    Comments[Line] = Comment;
  else if (Comments[Line].find(Comment) == std::string::npos)
    Comments[Line] += Comment;
} 

void RecoverExpressions::copyComments (std::map <unsigned int, std::string>
                                      CommentsIn) {
  for (auto I = CommentsIn.begin(), E = CommentsIn.end(); I != E; ++I) {
    addCommentToLine(I->second,I->first);
  }
} 

void RecoverExpressions::findRecursiveTasks() {
  for (auto &I: this->tasksList) {
    if (!I->isSafeForAnnotation()) 
      continue;
    if (RecursiveTask *RT = dyn_cast<RecursiveTask>(I)) { 
      isRecursive[RT->getRecursiveCall()->getCalledFunction()] = true;
      isFCall[RT->getRecursiveCall()] = true;
    }
    if (FunctionCallTask *FCT = dyn_cast<FunctionCallTask>(I)) {
      isFCall[FCT->getFunctionCall()] = true;
    }  
  }
}

void RecoverExpressions::insertCutoff(Function *F) {
  int start = st->getMinLineFunction(F) + 1;
  int end = getLastBranchLine(F);
  addCommentToLine("#pragma omp critical\ntaskminer_depth_cutoff++;\n", start);
  addCommentToLine("#pragma omp critical\ntaskminer_depth_cutoff--;\n", end);
}

int RecoverExpressions::getLineNo (Value *V) {
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

int RecoverExpressions::getLastBranchLine(Function *F) {
  int line = 0;
  for (auto B = F->begin(), BE = F->end(); B != BE; B++)
    for (auto II = B->begin(), IE = B->end(); II != IE; II++)
      if (isa<BranchInst>(&(*II))) {
        int l = getLineNo(II);
        if (l > line)
          line = l;
      }
  return line;
}

bool RecoverExpressions::isUniqueinLine(Instruction *I) {
  Function *F = I->getParent()->getParent();
  int line = getLineNo(I);
  for (auto B = F->begin(), BE = F->end(); B != BE; B++)
    for (auto II = B->begin(), IE = B->end(); II != IE; II++)
      if (isa<CallInst>(&(*II)))
        if ((line == getLineNo(II)) && ((&(*II)) != I)) {
          if (CallInst *CI = dyn_cast<CallInst>(II)) {
            Value *V = CI->getCalledValue();
            if (!isa<Function>(V))
              return false;
            Function *FF = cast<Function>(V);
            if (F->getName() == "llvm.dbg.declare")
              continue;
          }
          return false;
        }
  return true;
}

std::string RecoverExpressions::analyzeCallInst(CallInst *CI,
                                                const DataLayout *DT,
                                                RecoverPointerMD *RPM) {
  /*RecoverFunctionCall rfc;
  std::string computationName = "TM" + std::to_string(getIndex());
  rfc.setNAME(computationName);
  rfc.setRecoverNames(rn);
  rfc.initializeNewVars();
  
  rfc.annotataFunctionCall(CI, ptrRa, rp, aa, se, li, dt);*/
  Value *V = CI->getCalledValue();
  
  if (!isa<Function>(V)) { 
    return std::string();
  }
  Function *F = cast<Function>(V);
  std::string output = std::string();
  if (!F) {
    return output;
  }
  std::string priv = std::string();
  std::string shar = std::string();
  std::string name = F->getName();
  /*if (F->empty() || (name == "llvm.dbg.declare")) {
    valid = false;
    return output;
  } */
  // Define if this CALL INST is contained in the knowed tasks well
  // define by Task Miner
  bool isTask = false;
  bool hasTaskWait = false;
  bool isInsideLoop = true;
  Loop *L1 = nullptr;
  Loop *L2 = nullptr;
  this->liveIN.erase(this->liveIN.begin(), this->liveIN.end());
  this->liveOUT.erase(this->liveOUT.begin(), this->liveOUT.end());
  this->liveINOUT.erase(this->liveINOUT.begin(), this->liveINOUT.end());
  for (auto &I: this->tasksCalls) {
    if (CallInst *CII = dyn_cast<CallInst>(I)) {
      if (CI == CII) {
        std::string prag = "#pragma omp parallel\n";
        prag += "#pragma omp single\n";
        int line = getLineNo(CI);
        addCommentToBLine(prag, line);
        isTask = true;
        break;
      }
    }
  }
  if (isTask == false)
  for (auto &I: this->tasksList) {
     if (!I->isSafeForAnnotation())
       continue;
//    if (!(I->getCost().aboveThreshold()))
//      continue;
    if (FunctionCallTask *FCT = dyn_cast<FunctionCallTask>(I)) {
      if (FCT->getFunctionCall() == CI) {
        this->liveIN = FCT->getLiveIN();
        this->liveOUT = FCT->getLiveOUT();
        this->liveINOUT = FCT->getLiveINOUT();
        if (FCT->hasSyncBarrier()) {
          L1 = this->li->getLoopFor(CI->getParent());
          L2 = L1;
          while (L2->getParentLoop()) {
            L2 = L2->getParentLoop();
          }
        }
        annotateExternalLoop(CI);
        if (!isValidPrivateStr(I->getPrivateValues()))
          isTask = false;
        if (!isValidSharedStr(I->getSharedValues()))
           isTask = false;
        priv = getPrivateStr(I->getPrivateValues());
        shar = getSharedStr(I->getSharedValues());
        output = std::string();
        isTask = true;
        break;
      }
    }

    if (RecursiveTask *RT = dyn_cast<RecursiveTask>(I)) {

      if (RT->getRecursiveCall() == CI) {
        this->liveIN = RT->getLiveIN();
        this->liveOUT = RT->getLiveOUT();
        this->liveINOUT = RT->getLiveINOUT();
        isTask = true;
        hasTaskWait = RT->hasSyncBarrier();
        isInsideLoop = RT->insideLoop();
        insertCutoff(CI->getCalledFunction());
        if (!isValidPrivateStr(I->getPrivateValues()))
          isTask = false;
        if (!isValidSharedStr(I->getSharedValues()))
           isTask = false;
        priv = getPrivateStr(I->getPrivateValues());
        shar = getSharedStr(I->getSharedValues());
        /*if (RT->hasSyncBarrier()) {
          L1 = this->li->getLoopFor(CI->getParent());
          L2 = L1;
          if (L2)
            while (L2->getParentLoop()) {
              L2 = L2->getParentLoop();
          }
        }*/
        output = std::string();
        break;
      }
    }
  }
  if (isTask == false) {
    return output;
  }
  if (CI->getNumArgOperands() == 0) {
    if (priv != std::string())
      return priv;
    return "\n\n[UNDEF\nVALUE]\n\n";
  }
  std::map<Value*, std::string> strVal;
  for (unsigned int i = 0; i < CI->getNumArgOperands(); i++) {
    if (!isa<LoadInst>(CI->getArgOperand(i)) &&
        !isa<StoreInst>(CI->getArgOperand(i)) &&
        !isa<GetElementPtrInst>(CI->getArgOperand(i)) &&
        !isa<Argument>(CI->getArgOperand(i)) &&
        !isa<GlobalValue>(CI->getArgOperand(i)) &&
        !isa<AllocaInst>(CI->getArgOperand(i)) &&
        !isa<BitCastInst>(CI->getArgOperand(i))) {
      continue;
    }
     
    std::string str = analyzeValue(CI->getArgOperand(i), DT, RPM);
    if (str == std::string() || str == "0") {
      return std::string();
    }
    strVal[CI->getArgOperand(i)] = str;
  }
  annotateExternalLoop(CI, L1, L2);
  if (this->liveIN.size() != 0) {
    output += " depend(in:";
    bool isused = false;
    for (auto J = this->liveIN.begin(), JE = this->liveIN.end(); J != JE; J++) {
      if (isused)
        output += ",";
      isused = true;
      output += strVal[*J];
    }
    output += ")";
  }
  if (this->liveOUT.size() != 0) {
    output += " depend(out:";
    bool isused = false;
    for (auto J = this->liveOUT.begin(), JE = this->liveOUT.end(); J != JE; J++) {
      if (isused)
        output += ",";
      isused = true;
      output += strVal[*J];
    }
    output += ")";
  }
  if (this->liveINOUT.size() != 0) {
    output += " depend(inout:";
    bool isused = false;
    for (auto J = this->liveINOUT.begin(), JE = this->liveINOUT.end(); J != JE;
      J++) {
      if (isused)
        output += ",";
      isused = true;
      output += strVal[*J];
    } 
    output += ")";
  }
  if (hasTaskWait) {
    std::string wait = "#pragma omp taskwait\n";
    int line = getLineNo(CI);
    line++;
    addCommentToLine(wait, line);  
  }
  // HERE
  if (output == std::string()) {
    if ((priv != std::string()) && (shar != std::string()))
      return priv + shar;
    if (priv != std::string())
      return priv;
    if (shar != std::string())
      return shar;
    
    return "\n\n[UNDEF\nVALUE]\n\n";
  }
  output += priv;
  output += shar;
  return output;
}

std::string RecoverExpressions::analyzePointer(Value *V, const DataLayout *DT,
                                               RecoverPointerMD *RPM) {
    Instruction *I = cast<Instruction>(V);
    Value *BasePtrV = getPointerOperand(I);

    while (isa<LoadInst>(BasePtrV) || isa<GetElementPtrInst>(BasePtrV)) {
      if (LoadInst *LD = dyn_cast<LoadInst>(BasePtrV))
        BasePtrV = LD->getPointerOperand();

      if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(BasePtrV))
        BasePtrV = GEP->getPointerOperand();
    }

    int size = RPM->getSizeToValue (BasePtrV, DT);
    int var = -1;
    std::string result = RPM->getAccessStringMD(V, "", &var, DT);
    if (result.find("[") == std::string::npos) {
      return result;
    }

    if (!RPM->isValidPointer(BasePtrV, DT)) {
      valid = false;
      return std::string();
    }

    Type *tpy = V->getType();
    if ((tpy->getTypeID() == Type::HalfTyID) ||
        (tpy->getTypeID() == Type::FloatTyID) ||
        (tpy->getTypeID() == Type::DoubleTyID) ||
        (tpy->getTypeID() == Type::X86_FP80TyID) ||
        (tpy->getTypeID() == Type::FP128TyID) ||
        (tpy->getTypeID() == Type::PPC_FP128TyID) ||
        (tpy->getTypeID() == Type::IntegerTyID)) {
      return result;
    }

    if (!RPM->isValid()) {
      return std::string();
    }
    
    size = RPM->getSizeInBytes(size);
    std::string output = std::string();

    unsigned int i = 0;
    if (result.size() < 1) {
      return std::string();
    }

    if (result.find("[") == std::string::npos) {
      return result;
    }
    do {
      output += result[i];
      i++;
    } while((i < result.size()) && (result[i] != '['));

    if (result.size() > i)
      output += result[i];

    if (i > result.size()) {
      return std::string();
    }

    std::string newInfo = std::string();
    i++;

    bool needB = false;
    while((i < result.size()) && (result[i] != ']')) {
      if (result[i] == '[')
        needB = true;
      newInfo += result[i];
      i++;
    }
    if (needB)
      newInfo += result[i];   

    newInfo = "(" + newInfo + " / " + std::to_string(size) + ");\n";
    int var2 = -1;
    RPM->insertCommand(&var2, newInfo);
    bool needEE = (output.find("[") != std::string::npos);
    output = output + "TM" + std::to_string(getIndex());
    output += "[" + std::to_string(var2) + "]";
    if (needEE)
      output += "]";
    return output;

}

std::string RecoverExpressions::analyzeValue(Value *V, const DataLayout *DT,
RecoverPointerMD *RPM) {
  if (valid == false) {
    return std::string();
  }
  else if (CallInst *CI = dyn_cast<CallInst>(V)) {
    return analyzeCallInst(CI, DT, RPM);
  }
/*  else if ((isa<StoreInst>(V) || isa<LoadInst>(V)) ||
           isa<GetElementPtrInst>(V)) {
    return analyzePointer(V, DT, RPM);
  }*/
  else {
    int var = -1;
    std::string result = RPM->getAccessStringMD(V, "", &var, DT);
    if (!RPM->isValid())
      return std::string();
    if (result == std::string())
      return ("TM" + std::to_string(getIndex()) + "["+ std::to_string(var)) + "]";
    return result;
  }
}

void RecoverExpressions::annotateExternalLoop(Instruction *I, Loop *L1,
                                              Loop *L2) {
  /*if (!L1 && !L2) {
    annotateExternalLoop(I);
    return;
  }*/
  if (L2) {
    Region *R = rp->getRegionInfo().getRegionFor(I->getParent());
    if (L2->getHeader())
      R = rp->getRegionInfo().getRegionFor(L2->getHeader()); 
    if (!st->isSafetlyRegionLoops(R))
      return;
    int line = st->getStartRegionLoops(R).first;  
    std::string output = std::string();
    output += "#pragma omp parallel\n#pragma omp single\n";
    addCommentToLine(output, line);
  }  
  if (L1) {
    Region *R = rp->getRegionInfo().getRegionFor(I->getParent());
    if (L1->getHeader())
      R = rp->getRegionInfo().getRegionFor(L1->getHeader()); 
    if (!st->isSafetlyRegionLoops(R))
      return;
    int line = st->getEndRegionLoops(R).first;  
    line++;
    std::string output = std::string();
    output += "#pragma omp taskwait\n";
    addCommentToLine(output, line);
  }
}

void RecoverExpressions::annotateExternalLoop(Instruction *I) {
  Loop *LL = this->li->getLoopFor(I->getParent());
  if (!LL)
    return;
  Region *R = rp->getRegionInfo().getRegionFor(I->getParent()); 
  if (LL) {
    if (LL->getHeader())
      R = rp->getRegionInfo().getRegionFor(LL->getHeader());
  }
  Loop *L = this->li->getLoopFor(I->getParent());
  int line = getLineNo(L->getHeader()->getFirstNonPHIOrDbg());
  std::string output = std::string();
  output += "#pragma omp parallel\n#pragma omp single\n";
  addCommentToLine(output, line);
}

void RecoverExpressions::analyzeFunction(Function *F) {
  const DataLayout DT = F->getParent()->getDataLayout();

  std::map<Loop*, bool> loops;
  for (auto BB = F->begin(), BE = F->end(); BB != BE; BB++) {
    for (auto I = BB->begin(), IE = BB->end(); I != IE; I++) {
      if (isa<CallInst>(I)) {
        valid = true;
        RecoverPointerMD RPM;
        std::string computationName = "TM" + std::to_string(getNewIndex());
        RPM.setNAME(computationName);
        RPM.setRecoverNames(rn);
         RPM.initializeNewVars();
        std::string result = analyzeValue(I, &DT, &RPM);
        std::string check = std::string();
        if (result != std::string()) {
          std::string output = std::string();
          if (RPM.getIndex() > 0) {
            output += "long long int " + computationName + "[";
            output += std::to_string(RPM.getNewIndex()) + "];\n";
            output += RPM.getUniqueString();
            RPM.clearCommands();
            if (!RPM.isValid()) {
               continue;
            }
          }
          std::string prag = "cutoff_test = (taskminer_depth_cutoff < DEPTH_CUTOFF);\n";
          int line = getLineNo(I);
          addCommentToBLine(prag, line);
          if (isRecursive.count(I->getParent()->getParent()) > 0)
            check = " final(cutoff_test)";
          if (result != "\n\n[UNDEF\nVALUE]\n\n") {
            output += "#pragma omp task untied default(shared)" + result;
            output += check + "\n";
          }
          else
            output += "#pragma omp task untied default(shared)" + check + "\n";
          Region *R = rp->getRegionInfo().getRegionFor(BB);
          Loop *L = this->li->getLoopFor(I->getParent());
          if (L)
            annotateExternalLoop(I);
          /*Loop *L = this->li->getLoopFor(I->getParent());
          if (!isUniqueinLine(I)) {
            errs() << "Bug 1\n";
            continue;
          }
          if ((loops.count(L) == 0)) { //&& st->isSafetlyRegionLoops(R)) {
            annotateExternalLoop(I);
            loops[L] = true;
          }*/
          //if(st->isSafetlyRegionLoops(R)) {
            addCommentToLine(output, line);
          //}
        }
      }
    }
  }
}

Value *RecoverExpressions::getPointerOperand(Instruction *Inst) {
  if (LoadInst *Load = dyn_cast<LoadInst>(Inst))
    return Load->getPointerOperand();
  else if (StoreInst *Store = dyn_cast<StoreInst>(Inst))
    return Store->getPointerOperand();
  else if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(Inst))
    return GEP->getPointerOperand();

  return 0;
}

std::string RecoverExpressions::calculateLoopRangeCost(Loop *L,
                                                       std::string & rst) {
  std::string exp = "100";

    Module *M = L->getLoopPredecessor()->getParent()->getParent();;
    const DataLayout DT = DataLayout(M);
    Region *r = rp->getRegionInfo().getRegionFor(L->getHeader());
    Instruction *insertPt = L->getHeader()->getTerminator();
    
    std::vector<const SCEV *> ExprList;
    SCEVRangeBuilder rangeBuilder(se, DT, aa, li, dt, r, insertPt);
    Instruction *I = rangeBuilder.getInductionVariable(L);
    const SCEV* Limit = se->getSCEVAtScope(I, L);
    ExprList.push_back(Limit);
    if (!I)
      return exp;
    if (!rangeBuilder.canComputeBoundsFor(Limit)) {
      return std::string();
    }

    Value *low = rangeBuilder.getULowerBound(ExprList);
    Value *up = rangeBuilder.getUUpperBound(ExprList);
    int var1 = -1, var2 = -1;

    RecoverPointerMD RPM;
    std::string computationName = "TM" + std::to_string(getNewIndex());
    RPM.setNAME(computationName);
    RPM.setRecoverNames(rn);
    RPM.initializeNewVars();
  
    std::string lLimit = analyzeValue(low, &DT, &RPM);
    std::string uLimit = analyzeValue(up, &DT, &RPM);
    rst += "long long int " + computationName + "[";
    rst += std::to_string(RPM.getNewIndex()) + "];\n";
    rst += RPM.getUniqueString();
 
    if (var1 != -1)
      lLimit = (NAME + "[" + std::to_string(var1) + "]");
    if (var2 != -1)
      uLimit = (NAME + "[" + std::to_string(var2) + "]");
    exp = "(" + uLimit + " - " + lLimit + ")";
    if ((var2 == -1) && (lLimit == "0"))
      exp = uLimit;
  return exp;
}

int RecoverExpressions::getStatitcLoopCost(Loop *L) {
  int inst = 0;
  for (BasicBlock *BB : L->getBlocks()) {
    if (this->li->getLoopFor(BB) != L)
      continue;
    for (auto &I : BB->getInstList())
      inst++;
  }
  //L->dump();
  //errs() << "Cost = " << inst << "\n";
  return inst;
}

int RecoverExpressions::extracttmcnumb(std::string str) {
  int num = 0;
  for (int i = 3; i < str.size(); i++) {
    if ((str[i] == 't') && (str[i] == 'm') && (str[i] == 'c')) {
      for (;str[i] != ' ';i++) {
        num = num * 10;
        num += str[i] - '0';
        i = str.size();
      }
    }
  }
  return num;
}

std::string RecoverExpressions::getUniqueString(
                               std::map<int, std::string> & smbexp,       
                               std::map<int, std::vector<int> > & ref) {
  std::string result = std::string();
  std::vector<bool> nc(smbexp.size(), false);
  std::queue<int> q;
  // Insert all computed values in a queue structure.
  for (auto I = smbexp.begin(), IE = smbexp.end(); I != IE; I++)
    q.push(I->first);    
  // Iterate, and for each case, try to find one of the possible solutions:
  while (!q.empty()) {
    int id = q.front();
    q.pop();
    // Case 1: Without dependences.
    if (ref[id].size() == 0) {
       result = result + smbexp[id];
       nc[id] = true;
       continue; 
    }
    // Case 2: All dependences solved:
    bool valid = true;
    for (int i = 0; i < ref[id].size(); i++) {
      if (nc[ref[id][i]] == false) {
        valid = false;
        break;
      }
    }
    if (valid == true) {
       result = result + smbexp[id];
       nc[id] = true; 
    }
    // Case 3: Unsolved dependences:
    else {
      q.push(id);
    }
  }
  return result;
}

std::string RecoverExpressions::calculateTopLoopCost(Loop *L,
                                                    std::string & comp) {
  lid++;
  std::string exp = std::string();
  std::string rst = std::string();
  int inst = getStatitcLoopCost(L);
  exp += "int tm_cost" + std::to_string(lid) + " = (" + std::to_string(inst); 
  std::map<int, std::string> mexp;
  std::map<int, std::vector<int> > ref;
  int id = lid;
  for (Loop *SubLoop : L->getSubLoops()) {
    exp += " + tmc" + std::to_string(calculateLoopCost(SubLoop, mexp, ref, rst));
  }
  exp += ");\n";
  std::string solexp = getUniqueString(mexp, ref); 
  exp = rst + solexp + exp;
  comp = exp;
  ////L->dump();
 /// errs() << comp << "\n";
  return ("tm_cost" + std::to_string(id));
}

int RecoverExpressions::calculateLoopCost(Loop *L, std::map<int,
                                          std::string> & smbexp,
                                          std::map<int, std::vector<int> > & ref,
                                          std::string & rst) {
  std::string exp = std::string();
  std::string sexp = std::string();
  int inst = getStatitcLoopCost(L);
  lid++;
  int id = lid;
  exp += "int tmc" + std::to_string(id) + " = ";
  exp += calculateLoopRangeCost(L, rst) + " * (" + std::to_string(inst);
  for (Loop *SubLoop : L->getSubLoops()) { 
    int rid = calculateLoopCost(SubLoop, smbexp, ref, rst);
    ref[id].push_back(rid);
    exp += " + tmc" + std::to_string(rid); 
  }
  exp += ");\n";
  smbexp[id] = exp;
  return id;
}

bool RecoverExpressions::isValidPrivateStr(std::set<Value*> V) {
  bool hasInst = false;
  for (auto &It : V) {
    if (isa<Instruction>(It))
      hasInst = true;
  }
  if ((hasInst == false) || (V.size() == 0))
    return true;
  if ((V.size() > 0) && (getPrivateStr(V) != std::string()))
    return true;
  return false;
}

bool RecoverExpressions::isValidSharedStr(std::set<Value*> V) {
  bool hasInst = false;
  for (auto &It : V) {
    if (isa<Instruction>(It))
      hasInst = true;
  }
  if ((hasInst == false) || (V.size() == 0))
    return true;
  if ((V.size() > 0) && (getSharedStr(V) != std::string()))
    return true;
  return false;
}

std::string RecoverExpressions::getSharedStr(std::set<Value*> V) {
  if (V.size() == 0)
    return std::string();
  std::string str = " shared(";
  Instruction *I = nullptr;
  for (auto &It : V) {
    if (isa<Instruction>(It))
      I = cast<Instruction>(It);
  }
  if (I == nullptr) {
    return std::string();
  }
  Module *M = I->getParent()->getParent()->getParent();
  const DataLayout DT = DataLayout(M); 
  RecoverPointerMD RPM;
  std::string computationName = "TM" + std::to_string(getNewIndex());
  RPM.setNAME(computationName);
  RPM.setRecoverNames(rn);
  RPM.initializeNewVars();
  int i = 1;
  for (auto &It : V) {
    str += analyzeValue(It, &DT, &RPM); 
    if (i != V.size())
      str += ",";
    i++;
  }
  str += ")";
  return str;
}

std::string RecoverExpressions::getPrivateStr(std::set<Value*> V) {
  if (V.size() == 0)
    return std::string();
  std::string str = " firstprivate(";
  Instruction *I = nullptr;
  for (auto &It : V) {
    if (isa<Instruction>(It))
      I = cast<Instruction>(It);
  }
  if (I == nullptr) {
    return std::string();
  }
  Module *M = I->getParent()->getParent()->getParent();
  const DataLayout DT = DataLayout(M); 
  RecoverPointerMD RPM;
  std::string computationName = "TM" + std::to_string(getNewIndex());
  RPM.setNAME(computationName);
  RPM.setRecoverNames(rn);
  RPM.initializeNewVars();
  int i = 0;
  for (auto &It : V) {
    std::string strtmp = analyzeValue(It, &DT, &RPM); 
    i++;
    if (strtmp == std::string())
      continue;
    str += strtmp;
    if (i != V.size())
      str += ",";
  }
  str += ")";
  return str;
}

std::string RecoverExpressions::getDataPragmaRegion (
                           std::map<std::string, std::string> & vctLower,
                           std::map<std::string, std::string> & vctUpper,
                           std::map<std::string, char> & vctPtMA) {
  std::string result = std::string();
  std::vector<std::string> loads;
  std::vector<std::string> stores;
  std::vector<std::string> ldnsts; 
  for (auto I = vctPtMA.begin(), IE = vctPtMA.end(); I != IE; I++) {
    if (I->second == 2)
      stores.push_back(I->first);
    if (I->second == 1) {
      loads.push_back(I->first);
    }
    if (I->second == 3)
      ldnsts.push_back(I->first); 
  }
  // Create data copies - Host to Devide
  result += "#pragma omp task ";
  if (loads.size() != 0)
    result += "depend(in:";
  for (unsigned int i = 0, ie = loads.size(); i != ie; i++) {
    result += loads[i] + "[" + vctLower[loads[i]];
    result += ":" + vctUpper[loads[i]] + "]";
    if (i != (ie-1))
      result += ",";
  }
  if (loads.size() != 0)
    result += ") ";
  // Create data Copies - Device to Host
  if (stores.size() != 0)
      result += "depend(out:";
  for (unsigned int i = 0, ie = stores.size(); i != ie; i++) {
    result += stores[i] + "[" + vctLower[stores[i]];
    result += ":" + vctUpper[stores[i]] + "]";
    if (i != (ie-1))
      result += ",";
  }
  if (stores.size() != 0)
    result += ")";

  if (ldnsts.size() != 0)
    result += "depend(inout: ";
  for (unsigned int i = 0, ie = ldnsts.size(); i != ie; i++) {
    result += ldnsts[i] + "[" + vctLower[ldnsts[i]];
    result += ":" + vctUpper[ldnsts[i]] + "]";
    if (i != (ie-1))
      result += ",";
  }
  if (ldnsts.size() != 0)
    result += ")";
  return result;
}

bool RecoverExpressions::analyzeLoop (Loop* L, int Line, int LastLine,
                                        PtrRangeAnalysis *ptrRA, 
                                        RegionInfoPass *rp, AliasAnalysis *aa,
                                        ScalarEvolution *se, LoopInfo *li,
                                        DominatorTree *dt, std::string priv) {
  if (!this->ptrRea->canSelectInfo(L)) {
    return false;
  }
  // Initilize The Analisys with Default Values.
  Module *M = L->getLoopPredecessor()->getParent()->getParent();
  const DataLayout DT = DataLayout(M);
  std::map<Value*, std::pair<Value*, Value*> > pointerBounds;
  std::string expression = std::string();
  std::string expressionEnd = std::string();
//  RecoverPointerMD RPM;
  RecoverCode RC;
  std::string computationName = "TM" + std::to_string(getNewIndex());
/*  RPM.setNAME(computationName);
  RPM.setRecoverNames(rn);
  RPM.initializeNewVars();
 
  Region *r = RPM.regionofBasicBlock((L->getLoopPreheader()), rp);
*/
  RC.setNAME(computationName);
  RC.setRecoverNames(rn);
  RC.initializeNewVars();
 
//  Region *r = RPM.regionofBasicBlock((L->getLoopPreheader()), rp);
  Region *r = RC.regionofBasicBlock((L->getLoopPreheader()), rp);

  if (!ptrRA->RegionsRangeData[r].HasFullSideEffectInfo)
    Region *r = RC.regionofBasicBlock((L->getHeader()), rp);
//    Region *r = RPM.regionofBasicBlock((L->getHeader()), rp);

  if (!ptrRA->RegionsRangeData[r].HasFullSideEffectInfo) {
    return false;
  }
  for (auto BB = L->block_begin(), BE = L->block_end(); BB != BE; BB++) {
//    Region *rr = RPM.regionofBasicBlock(*BB, rp);
    Region *rr = RC.regionofBasicBlock(*BB, rp); 
    if (r != rr) {
      r = rr;
      break;
    }
  }
  Instruction *insertPt = r->getEntry()->getTerminator();
  SCEVRangeBuilder rangeBuilder(se, DT, aa, li, dt, r, insertPt);
  Loop *LParent = this->li->getLoopFor(L->getLoopPreheader());
  rangeBuilder.setLoop(LParent);
  // Generate and store both bounds for each base pointer in the region.
  for (auto& pair : ptrRA->RegionsRangeData[r].BasePtrsData) {
//    if (RPM.pointerDclInsideLoop(L,pair.first))
    if (RC.pointerDclInsideLoop(L,pair.first))
      continue;
    // Adds "sizeof(element)" to the upper bound of a pointer, so it gives us
    // the address of the first byte after the memory region.
/*    Value *low = rangeBuilder.getULowerBound(pair.second.AccessFunctions);
    if (rangeBuilder.isLoopUsed())
      low = rangeBuilder.addSymbExp(LParent, low, false);
    Value *up = rangeBuilder.getUUpperBound(pair.second.AccessFunctions);
    if (rangeBuilder.isLoopUsed())
      up = rangeBuilder.addSymbExp(LParent, up, true); 
//    up = rangeBuilder.stretchPtrUpperBound(pair.first, up);*/
    Value *low = this->ptrRea->getIndVarBounds(LParent, pair.first).first;
    Value *up = this->ptrRea->getIndVarBounds(LParent, pair.first).second;
    pointerBounds.insert(std::make_pair(pair.first, std::make_pair(low, up)));
  }

  std::map<std::string, std::string> vctLower;
  std::map<std::string, std::string> vctUpper;
  std::map<std::string, char> vctPtMA;
  std::map<std::string, Value*> vctPtr;

  for (auto It = pointerBounds.begin(), EIt = pointerBounds.end(); It != EIt;
       ++It) {
    RecoverNames::VarNames nameF = rn->getNameofValue(It->first);
    int var1 = -1, var2 = -1;
  //  std::string lLimit = RPM.getAccessString(It->second.first, nameF.nameInFile, &var1, &DT);
  //  std::string uLimit = RPM.getAccessString(It->second.second, nameF.nameInFile, &var2, &DT);
//    std::string lLimit = RPM.getAccessExpression(It->first, It->second.first, &DT, false);
//    std::string uLimit = RPM.getAccessExpression(It->first, It->second.second, &DT, true);
    std::string lLimit = RC.getAccessExpression(It->first, It->second.first, &DT, false);
    std::string uLimit = RC.getAccessExpression(It->first, It->second.second, &DT, true);
    std::string olLimit = std::string();
    std::string oSize = std::string();
    //RPM.generateCorrectUB(lLimit, uLimit, olLimit, oSize);
    vctLower[nameF.nameInFile] = lLimit;
    vctUpper[nameF.nameInFile] = uLimit;
    vctPtMA[nameF.nameInFile] = ptrRA->getPointerAcessType(L, It->first);
    vctPtr[nameF.nameInFile] = It->first;
    if (!RC.isValid()) { 
//    if (!RPM.isValid()) { 
      errs() << "[TRANSFER-PRAGMA-INSERTION] WARNING: unable to generate C " <<
        " code for bounds of pointer: " << (nameF.nameInFile.empty() ?
        "<unable to recover pointer name>" : nameF.nameInFile) << "\n";
      It->first->dump();
//      return RPM.isValid();
      return RC.isValid();
    }
  }
  std::string output = "{\n";
  int thld = ((RUNTIME_COST * THRESHOLD) * N_WORKERS);
  std::string cmpadd = std::string();
  std::string cmpif = " final(" + calculateTopLoopCost(L, cmpadd);
  cmpif += " > " + std::to_string(thld) + ")";// + std::to_string(thld) + ")";
  output += cmpadd;
 
/*  if (RPM.getIndex() > 0) {
    output += "long long int " + computationName + "[";
    output += std::to_string(RPM.getNewIndex()) + "];\n";
    output += RPM.getUniqueString();
    RPM.clearCommands();
  }*/
  if (RC.getIndex() > 0) {
    output += "long long int " + computationName + "[";
    output += std::to_string(RC.getNewIndex()) + "];\n";
    output += RC.getUniqueString();
    RC.clearCommands();
  }
 
  // HERE
  output += getDataPragmaRegion(vctLower, vctUpper, vctPtMA);
  output += priv + " ";
  output += cmpif + "\n{\n";
  addCommentToLine(output, Line);
  addCommentToLine("}\n}\n", LastLine); 
  //annotateExternalLoop((L->getHeader()->begin()));
  return true;
}

bool RecoverExpressions::analyzeTopLoop (Loop* L, int Line, int LastLine,
                                        PtrRangeAnalysis *ptrRA, 
                                        RegionInfoPass *rp, AliasAnalysis *aa,
                                        ScalarEvolution *se, LoopInfo *li,
                                        DominatorTree *dt, std::string priv) {
  // Initilize The Analisys with Default Values.
  Module *M = L->getLoopPredecessor()->getParent()->getParent();
  const DataLayout DT = DataLayout(M);
  RecoverPointerMD RPM;
  std::string computationName = "TM" + std::to_string(getNewIndex());
  RPM.setNAME(computationName);
  RPM.setRecoverNames(rn);
  RPM.initializeNewVars();
 
  Region *r = RPM.regionofBasicBlock((L->getLoopPreheader()), rp);

  if (!ptrRA->RegionsRangeData[r].HasFullSideEffectInfo)
    r = RPM.regionofBasicBlock((L->getHeader()), rp);
 
  if (!ptrRA->RegionsRangeData[r].HasFullSideEffectInfo) {
    return false;
  }
  
  bool exLoop = false;
  //errs() << "Top loop is:\n";
  //L->dump();
  for (Loop *SubLoop : L->getSubLoops()) {
    Region *R = this->rp->getRegionInfo().getRegionFor(SubLoop->getHeader());
      if (!R)
        continue;

      if (mapped.count(SubLoop) > 0)
        continue;
      mapped[SubLoop] = true;

    int start = this->st->getStartRegionLoops(R).first;
    int end = this->st->getEndRegionLoops(R).first;
    //errs() << "Sub Loop is:\n";
    //SubLoop->dump();
    if (analyzeLoop(SubLoop, start, end, this->ptrRa, this->rp, this->aa,
                    this->se, this->li, this->dt, priv) == true) {
      numRA++;
      exLoop = true;
    }
  }
  if (exLoop)
    annotateExternalLoop((L->getHeader()->begin()));
}

void RecoverExpressions::getTaskRegions() {
  bbsRegion.erase(bbsRegion.begin(), bbsRegion.end());
  for (auto &I: this->tasksList) {
//    if (!(I->getCost().aboveThreshold()))
//      continue;
    
    if (!I->isSafeForAnnotation())
      continue;
    if (RegionTask *RT = dyn_cast<RegionTask>(I)) {
        BasicBlock *BB = RT->getHeaderBB();
        Loop *L = li->getLoopFor(BB);
        Region *R = this->rp->getRegionInfo().getRegionFor(BB);
        if (!R)
          continue;
        if (mapped.count(L) > 0)
          continue;
        mapped[L] = true;
        int start = this->st->getStartRegionLoops(R).first;
        int end = this->st->getEndRegionLoops(R).first;
        std::string priv = getPrivateStr(I->getPrivateValues());
        analyzeTopLoop(L, start, end, this->ptrRa, this->rp, this->aa, this->se,
                    this->li, this->dt, priv);
    }
  }
}

bool RecoverExpressions::runOnFunction(Function &F) {
  this->li = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  this->rp = &getAnalysis<RegionInfoPass>();
  this->aa = &getAnalysis<AliasAnalysis>();
  this->se = &getAnalysis<ScalarEvolution>();
  this->dt = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  this->rn = &getAnalysis<RecoverNames>();
  this->rr = &getAnalysis<RegionReconstructor>();
  this->st = &getAnalysis<ScopeTree>();
  this->ptrRa = &getAnalysis<PtrRangeAnalysis>();
  this->ptrRea = &getAnalysis<PtrRRangeAnalysis>();

  lid = 0;
  findRecursiveTasks();
  
  std::string header = "#include <omp.h>\n";
  header += "#ifndef taskminerutils\n";
  header += "#define taskminerutils\n"; 
  header += "static int taskminer_depth_cutoff = 0;\n";
  header += "#define DEPTH_CUTOFF omp_get_num_threads()\n";
  header += "extern char cutoff_test;\n";
  header += "#endif\n"; 
  addCommentToLine(header, 1);
  index = 0;
  analyzeFunction(&F);
  getTaskRegions();

  return true;
}

char RecoverExpressions::ID = 0;
static RegisterPass<RecoverExpressions> Z("recoverExpressions",
"Recover Expressions to the source File.");

//===------------------------ recoverExpressions.cpp ------------------------===//
