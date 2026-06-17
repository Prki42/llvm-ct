#include "ArgDepAnalysis.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace ct {

AnalysisKey ArgDepAnalysis::Key;

ArgDepResult ArgDepAnalysis::run(Function &F, FunctionAnalysisManager &) {
  errs() << "ArgDepAnalysis visiting: " << F.getName() << "\n";
  return {};
}

} // namespace ct
