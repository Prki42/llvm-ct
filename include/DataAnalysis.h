#ifndef DATA_ANALYSIS_H
#define DATA_ANALYSIS_H

#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Type.h"

#include <cstdint>
#include <unordered_map>

namespace ct {

// Describes a statically-known memory region rooted at an alloca.
// Interval [OffsetLow, OffsetHigh) is in bytes relative to Base.
// Size is the size of the pointed-to type (Form) at this level -
// distinct from the interval width once we handle GEPs.
// Base is AllocaInst for local variables, or Argument for pointer parameters.
// For arguments the allocation extent is unknown - Size and Form reflect only
// the type at the access site, not the full object.
struct MemoryObject {
  llvm::Value *Base;       // root: AllocaInst or Argument
  llvm::Function *Context; // where the access occurs
  int64_t OffsetLow;       // low end of the interval (inclusive)
  int64_t OffsetHigh;      // high end of the interval (exclusive)
  uint64_t Size;           // size of the pointed-to type (Form) at this level
  llvm::Type *Form;        // type of the pointed-to object at this level
};

// Maps each sensitive load/store to the descriptor of the memory object
// its pointer traces back to.
struct DataResult {
  std::unordered_map<llvm::Instruction *, MemoryObject> Objects;
};

class DataAnalysis : public llvm::AnalysisInfoMixin<DataAnalysis> {
  friend llvm::AnalysisInfoMixin<DataAnalysis>;
  static llvm::AnalysisKey Key;

public:
  using Result = DataResult;
  Result run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM);
};

class DataPrinterPass : public llvm::PassInfoMixin<DataPrinterPass> {
public:
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &FAM);
  static bool isRequired() { return true; }
};

} // namespace ct

#endif
