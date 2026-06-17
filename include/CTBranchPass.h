#ifndef CT_BRANCH_PASS_H
#define CT_BRANCH_PASS_H

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"

namespace ct {

class CTBranchPass : public llvm::PassInfoMixin<CTBranchPass> {
public:
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &FAM);
  static bool isRequired() { return true; };
};

} // namespace ct

#endif
