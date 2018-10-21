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

#include "llvm/IR/DIBuilder.h" 
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/ADT/Statistic.h"

#include "extractSourceData.h"

using namespace llvm;
using namespace std;

int ExtractSourceData::getLineNo (Value *V) {
  if (!V)
    return -1;
  if (Instruction *I = dyn_cast<Instruction>(V))
    if (I)
      if (MDNode *N = I->getMetadata("dbg"))
        if (N)
          if (DILocation *DI = dyn_cast<DILocation>(N))
            return DI->getLine();
  return -1;
}

std::string ExtractSourceData::getFileName(Function *F) {
  for (auto BB = F->begin(), BE = F->end(); BB != BE; BB++)
    for (auto I = BB->begin(), IE = BB->end(); I != IE; I++) {
      MDNode *Var = I->getMetadata("dbg");
      if (Var)
        if (DILocation *DL = dyn_cast<DILocation>(Var))
          return DL->getFilename();
    }
  return std::string();
}

int ExtractSourceData::getFinish(Instruction *I) {
  std::string name = getFileName(I->getParent()->getParent());
  if (info.count(name) == 0)
    analyzeFunction(I->getParent()->getParent());
  int line = getLineNo(I);
  if (info[name].sourceData.count(line) == 0)
    return line;
  return info[name].sourceData[line];
}

void ExtractSourceData::analyzeFunction(Function *F) {
  std::string name = getFileName(F);
  if (name == std::string()) {
    return;
  }
  if (info.count(name) != 0) {
    return;
  }
  std::fstream Infile;
  Infile.open(name.c_str(), std::ios::in);
  if (!Infile)
    return;

  long long int cont = 1;
  long long int startStm = -1;
  bool open = false;
  while (!Infile.eof()) {
    std::string Line = std::string();
    std::getline(Infile, Line);
    for (int i = 0; i < Line.size(); i++) {
      if ((open == true) && (Line[i] != '*'))
        continue;
      if (Line[i] == ' ' || Line[i] == '\t')
        continue;
      if (open == true) {
        if (i < Line.size())
          i++;
        if (Line[i] == '/') {
          open = false;
          for (int j = startStm; j < cont; j++)
            info[name].sourceData[j] = cont;
          startStm = cont;
        }
      }
      if (Line[i] == '#') {
        info[name].sourceData[cont] = cont;
        break;
      }
      if ((i > 0) && (Line[(i-1)] == '/')) {
        if (Line[i] == '/') {
          info[name].sourceData[cont] = cont;  
          break;
        }
        if (Line[i] == '*') {
          startStm = cont;
          open = true;
          continue;
        }
      }
      if (Line[i] == ';') {
        for (int j = startStm; j < cont; j++)
          info[name].sourceData[j] = cont;
         startStm = cont;
      }
    }
    cont++;
  }
}

//===------------------------ recoverExpressions.cpp ------------------------===//
