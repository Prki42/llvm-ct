#ifndef ARG_DEP_ANALYSIS_H
#define ARG_DEP_ANALYSIS_H

#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"

#include <unordered_set>

namespace ct {

struct ArgDepResult {
  std::unordered_set<llvm::BranchInst *> MarkedBranches;
  std::unordered_set<llvm::Value *> MarkedValues;
};

class ArgDepAnalysis : public llvm::AnalysisInfoMixin<ArgDepAnalysis> {
  friend llvm::AnalysisInfoMixin<ArgDepAnalysis>;
  static llvm::AnalysisKey Key;

public:
  using Result = ArgDepResult;

  Result run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM);
};

class ArgDepPrinterPass : public llvm::PassInfoMixin<ArgDepPrinterPass> {
public:
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &FAM);
  static bool isRequired() { return true; };
};

} // namespace ct

#endif
