#include "CTBranchPass.h"
#include "ArgDepAnalysis.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace ct {

PreservedAnalyses CTBranchPass::run(Function &F, FunctionAnalysisManager &FAM) {
  auto _ = FAM.getResult<ArgDepAnalysis>(F);
  errs() << "CTBranchPass visiting: " << F.getName() << "\n";
  return PreservedAnalyses::all();
}

} // namespace ct
