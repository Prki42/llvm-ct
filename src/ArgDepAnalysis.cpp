#include "ArgDepAnalysis.h"
#include "llvm/IR/Analysis.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"

#include <queue>

using namespace llvm;

namespace ct {

AnalysisKey ArgDepAnalysis::Key;

ArgDepResult ArgDepAnalysis::run(Function &F, FunctionAnalysisManager &) {
  ArgDepResult res;
  std::queue<llvm::Value *> markedValues;

  for (auto &Arg : F.args()) {
    res.MarkedValues.insert(&Arg);
    markedValues.push(&Arg);
  }

  // gathering all vulnerable values!
  while (!markedValues.empty()) {
    auto currValue = markedValues.front();
    markedValues.pop();

    for (auto *User : currValue->users()) {
      auto *I = llvm::dyn_cast<llvm::Instruction>(User);
      if (I) {
        if (res.MarkedValues.insert(I).second) {
          markedValues.push(I);
        }
      }
    }
  }

  // gathering all vulnerable branches
  for (auto &BB : F) {
    auto *BI = llvm::dyn_cast<llvm::BranchInst>(BB.getTerminator());
    if (BI && BI->isConditional()) {
      if (res.MarkedValues.count(BI->getCondition())) {
        res.MarkedBranches.insert(BI);
      }
    }
  }
  return res;
}

PreservedAnalyses ArgDepPrinterPass::run(llvm::Function &F,
                                         llvm::FunctionAnalysisManager &FAM) {
  // We get the analysis results
  auto &Res = FAM.getResult<ArgDepAnalysis>(F);

  llvm::errs() << "\n--- ArgDepAnalysis Debug: " << F.getName() << " ---\n";

  // We write out all the branches
  llvm::errs() << "Found " << Res.MarkedBranches.size()
               << " conditionals which need CFL:\n";

  for (auto *BI : Res.MarkedBranches) {
    llvm::errs() << "  [!] Conditional: " << *BI << "\n";
  }

  // We write out all the values which are marked
  llvm::errs() << "Number of values: " << Res.MarkedValues.size()
               << "\n";
  // As well as all the instructions that contain the marked values
  for (auto *V : Res.MarkedValues) {
    if (auto *I = llvm::dyn_cast<llvm::Instruction>(V)) {
      llvm::errs() << "    Marked Value: " << *I << "\n";
    }
  }
  llvm::errs() << "------------------------------------------\n";

  return llvm::PreservedAnalyses::all();
}

} // namespace ct
