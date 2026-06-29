#ifndef CT_DATA_PASS_H
#define CT_DATA_PASS_H

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"

namespace ct {

class CTDataPass : public llvm::PassInfoMixin<CTDataPass> {
public:
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &FAM);
  static bool isRequired() { return true; }
};

} // namespace ct

#endif
