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

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/LoopInfo.h"

namespace llvm {

class DominanceFrontier;
struct PostDominatorTree;
class Value;
class Instruction;
class LoopInfo;

class ExtractSourceData {

  private:

  //===---------------------------------------------------------------------===
  //                              Data Structs
  //===---------------------------------------------------------------------===
  typedef struct FileInfo {
    std::string fileName;
    std::map<int, int> sourceData;
  } FileInfo;

  std::map<std::string, FileInfo> info;
  //===---------------------------------------------------------------------===  

  int getLineNo(Value *V);

  std::string getFileName(Function *F);

  void analyzeFunction(Function *F);

  void processFile(std::string fileName);

  public:

  //===---------------------------------------------------------------------===
  //                              Data Structs
  //===---------------------------------------------------------------------===
  //===---------------------------------------------------------------------===

  // Return the line with the next ";"
  int getFinish(Instruction *I);

};

}

//===---------------------- recoverExpressions.h --------------------------===//
