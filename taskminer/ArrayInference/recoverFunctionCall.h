//===------------------------ recoverFunctionCall.h -----------------------===//
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
#include "llvm/Analysis/RegionInfo.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/LoopInfo.h"
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

#ifndef myutils2
#define myutil2
#include "recoverPointerMD.h"

#endif

#ifndef myutils
#define myutils

//include "recoverCode.h"
#include "../ScopeTree/ScopeTree.h"
#include "PtrRangeAnalysis.h"
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

class RecoverFunctionCall : public RecoverPointerMD {

  protected:

  //===---------------------------------------------------------------------===
  //                              Data Structs
  //===---------------------------------------------------------------------===
  typedef struct DataLimits {
    std::string lowerBound;
    std::string upperBound;
    bool hasParent;
    Loop *Parent;
    std::vector<std::string> subLoopsLimits;
  } DataLimits;


  // Data structs to identify a instruction with the command with C/C++ sintaxy:
  std::string computationRef;

  std::map<std::string, bool> simbols;
  
  std::map<std::string, std::string> newSimbols;
  std::map<std::string, std::string> oldSimbols;

  Value *contextFunctionCaller;
  //===---------------------------------------------------------------------===

  public:

  //===---------------------------------------------------------------------===  
  //                              Data Structs                                   
  //===---------------------------------------------------------------------===

  //===---------------------------------------------------------------------===

   int getLineNo (Value *V);

  // Map to a <Intern Function Simbol, Caller Simbol>
  void findAllArgumentsAndGlobalValues (CallInst *CI,
                                std::map<std::string,std::string> & recovered);
  void getSimbolsContext (CallInst *CI,
                          std::map<std::string, std::string> & sbls);

  void findAllKnowedSimbols ();

  void ReplaceAllSimbols ();
  
  bool isReplacebleSimbols ();

  bool isValidToContext(Value *V);

  void getSubLoopsLimits (std::map<Loop*, DataLimits> loopsLimits);

  void collectLoopInfo (Loop *L, Loop *Parent, CallInst *CI,
                                PtrRangeAnalysis *ptrRA, 
                                RegionInfoPass *rp, AliasAnalysis *aa,
                                ScalarEvolution *se, LoopInfo *li,
                                DominatorTree *dt,
                                std::map<Loop*, DataLimits> & loopsLimits);

  void annotataFunctionCall (CallInst *CI, PtrRangeAnalysis *ptrRA, 
                                        RegionInfoPass *rp, AliasAnalysis *aa,
                                        ScalarEvolution *se, LoopInfo *li,
                                        DominatorTree *dt);
};

}

//===----------------------------- recoverCode.h --------------------------===//
