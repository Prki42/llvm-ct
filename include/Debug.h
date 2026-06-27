#ifndef DEBUG_H
#define DEBUG_H

#include "llvm/Support/Debug.h"

#ifndef NDEBUG
#include "llvm/Support/CommandLine.h"

extern llvm::cl::opt<bool> CTVerbose;

// redefine LLVM_DEBUG
#undef LLVM_DEBUG
#define LLVM_DEBUG(X)                                                          \
  do {                                                                         \
    if (CTVerbose) {                                                           \
      X;                                                                       \
    }                                                                          \
  } while (0)
#endif

#endif
