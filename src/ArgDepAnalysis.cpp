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
  std::queue<llvm::Value *> valueWorklist;

  auto mark = [&res, &valueWorklist](Value *to_be_marked) {
    // avoid adding already marked instructions to worklist
    if (res.MarkedValues.insert(to_be_marked).second) {
      valueWorklist.push(to_be_marked);
    }
  };

  for (auto &Arg : F.args()) {
    res.MarkedValues.insert(&Arg);
    valueWorklist.push(&Arg);
  }

  // gathering all vulnerable values
  while (!valueWorklist.empty()) {
    auto currValue = valueWorklist.front();
    valueWorklist.pop();

    for (auto *User : currValue->users()) {
      if (auto *I = llvm::dyn_cast<llvm::Instruction>(User)) {

        // handle store separately
        if (auto *store_inst = llvm::dyn_cast<llvm::StoreInst>(I)) {
          auto *val_op = store_inst->getValueOperand();
          // if value operand is marked, mark the ptr operand
          if (res.MarkedValues.count(val_op)) {
            auto ptr_op = store_inst->getPointerOperand();
            mark(ptr_op);
          }
          continue;
        }

        // all other instructions
        mark(I);
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
  llvm::errs() << "Number of values: " << Res.MarkedValues.size() << "\n";
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
