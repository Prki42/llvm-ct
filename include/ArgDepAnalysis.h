#ifndef ARG_DEP_ANALYSIS_H
#define ARG_DEP_ANALYSIS_H

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"

namespace ct {

struct ArgDepResult {};

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
