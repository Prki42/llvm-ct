#include "ArgDepAnalysis.h"
#include "llvm/IR/Analysis.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace ct {

AnalysisKey ArgDepAnalysis::Key;

ArgDepResult ArgDepAnalysis::run(Function &F, FunctionAnalysisManager &) {
  errs() << "ArgDepAnalysis visiting: " << F.getName() << "\n";
  return {};
}

PreservedAnalyses ArgDepPrinterPass::run(llvm::Function &F,
                                         llvm::FunctionAnalysisManager &FAM) {
  auto _ = FAM.getResult<ArgDepAnalysis>(F);
  errs() << "ArgDepPrinterPass visiting: " << F.getName() << "\n";
  return PreservedAnalyses::all();
}

} // namespace ct
