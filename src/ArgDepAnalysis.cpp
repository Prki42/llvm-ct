#include "llvm/IR/Analysis.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

#include "ArgDepAnalysis.h"

#include <algorithm>
#include <cassert>
#include <queue>

#define DEBUG_TYPE "ct-analysis"
#include "Debug.h"

using namespace llvm;

namespace ct {

AnalysisKey ArgDepAnalysis::Key;

ArgDepResult ArgDepAnalysis::run(Function &F, FunctionAnalysisManager &) {
  ArgDepResult res;
  std::queue<Value *> valueWorklist;

  auto mark = [&res, &valueWorklist](Value *to_be_marked) {
    // only mark those that are of the form "%x = ..."
    if (auto *I = dyn_cast<Instruction>(to_be_marked))
      if (I->getType()->isVoidTy()) {
        // sanity check - can't have no uses if it's void type
        assert(I->use_empty() && "Found void type with some uses?");
        return;
      }

    // avoid adding already marked instructions to worklist
    if (res.MarkedValues.insert(to_be_marked).second) {
      valueWorklist.push(to_be_marked);
    }
  };

  LLVM_DEBUG(dbgs() << "Adding arguments:\n");
  for (auto &Arg : F.args()) {
    LLVM_DEBUG(dbgs() << "  " << Arg.getArgNo() << ": " << Arg << "\n");
    mark(&Arg);
  }
  LLVM_DEBUG(dbgs() << "\n");

  // gathering all vulnerable values
  LLVM_DEBUG(dbgs() << "Traversing uses\n");
  while (!valueWorklist.empty()) {
    auto currValue = valueWorklist.front();
    valueWorklist.pop();

    LLVM_DEBUG(dbgs() << "  Popped: " << *currValue << "\n");
    LLVM_DEBUG(dbgs() << "  uses:" << "\n");

    for (auto *User : currValue->users()) {
      if (auto *I = dyn_cast<Instruction>(User)) {
        LLVM_DEBUG(dbgs() << "    " << *I << "\n");

        // handle store separately
        if (auto *store_inst = dyn_cast<StoreInst>(I)) {
          // if value operand is marked (current value), mark the ptr operand
          if (store_inst->getValueOperand() == currValue)
            mark(store_inst->getPointerOperand());
          continue;
        }

        // all other instructions
        mark(I);
      }
    }
  }
  LLVM_DEBUG(dbgs() << "\nEnd of traversal\n");

  // gathering all vulnerable branches
  LLVM_DEBUG(dbgs() << "Marking branches:\n");
  for (auto &BB : F) {
    auto *BI = dyn_cast<BranchInst>(BB.getTerminator());
    if (BI && BI->isConditional()) {
      if (res.MarkedValues.count(BI->getCondition())) {
        LLVM_DEBUG(dbgs() << "  Marked " << *BI << "\n");
        res.MarkedBranches.insert(BI);
      }
    }
  }

  return res;
}

PreservedAnalyses ArgDepPrinterPass::run(Function &F,
                                         FunctionAnalysisManager &FAM) {
  auto &Res = FAM.getResult<ArgDepAnalysis>(F);

  // for deterministic tests, sort the results by their posistion in IR
  auto cmpByPosition = [&F](Instruction *A, Instruction *B) {
    if (A->getParent() == B->getParent())
      return A->comesBefore(B);
    // if they're not in the same block, find which block is first
    for (auto &BB : F) {
      if (&BB == A->getParent())
        return true;
      if (&BB == B->getParent())
        return false;
    }
    LLVM_DEBUG(dbgs() << "Could not compare: " << *A << " and " << *B << "\n");
    llvm_unreachable("Instructions not comparable");
  };

  std::vector<Instruction *> sorted_values;
  std::vector<BranchInst *> sorted_branches;

  for (auto *V : Res.MarkedValues) {
    if (auto *I = dyn_cast<Instruction>(V))
      sorted_values.push_back(I);
  }
  for (auto *I : Res.MarkedBranches) {
    sorted_branches.push_back(I);
  }

  std::sort(sorted_values.begin(), sorted_values.end(), cmpByPosition);
  std::sort(sorted_branches.begin(), sorted_branches.end(), cmpByPosition);

  // print sorted results

  outs() << "ArgDep for " << F.getName() << ":\n";

  outs() << "  Branches:\n";
  for (auto *BI : sorted_branches) {
    outs() << "    " << *BI << "\n";
  }

  outs() << "  Values:\n";
  for (auto &Arg : F.args()) {
    assert(Res.MarkedValues.count(&Arg) && "argument nor marked");
    outs() << "    " << Arg << "\n";
  }
  for (auto *V : sorted_values) {
    if (auto *I = dyn_cast<Instruction>(V))
      outs() << "    " << *I << "\n";
  }
  return PreservedAnalyses::all();
}

} // namespace ct
