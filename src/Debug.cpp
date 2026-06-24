#include "Debug.h"

#ifndef NDEBUG
llvm::cl::opt<bool> CTVerbose("ct-verbose",
                              llvm::cl::desc("Verbose output for ct plugin"),
                              llvm::cl::init(false));
#endif
