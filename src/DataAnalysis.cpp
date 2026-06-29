#include "llvm/IR/Argument.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"

#include "ArgDepAnalysis.h"
#include "DataAnalysis.h"

#include <algorithm>
#include <vector>

using namespace llvm;

namespace ct {

// TODO add debug logs...

AnalysisKey DataAnalysis::Key;

// Traces a pointer back through GEPs and bitcasts to its root.
// Returns an AllocaInst or a pointer-typed Argument.
// Returns nullptr if the root cannot be statically determined.
static Value *traceToBase(Value *Ptr) {
  while (Ptr) {
    if (isa<AllocaInst>(Ptr) || isa<Argument>(Ptr))
      return Ptr;
    if (auto *GEP = dyn_cast<GetElementPtrInst>(Ptr)) {
      Ptr = GEP->getPointerOperand();
      continue;
    }
    if (auto *BC = dyn_cast<BitCastInst>(Ptr)) {
      Ptr = BC->getOperand(0);
      continue;
    }
    break;
  }
  return nullptr;
}

DataResult DataAnalysis::run(Function &F, FunctionAnalysisManager &FAM) {
  DataResult Res;
  const DataLayout &DL = F.getParent()->getDataLayout();
  // Take the argument masked analysis, since we only care about the
  // instructions that are dependent on marked values
  auto &Dep = FAM.getResult<ArgDepAnalysis>(F);

  for (auto &BB : F) {
    for (auto &I : BB) {
      Value *Ptr = nullptr;
      Type *AccessType = nullptr;

      if (auto *Load = dyn_cast<LoadInst>(&I)) {
        Ptr = Load->getPointerOperand();
        AccessType = Load->getType();
      } else if (auto *Store = dyn_cast<StoreInst>(&I)) {
        Ptr = Store->getPointerOperand();
        AccessType = Store->getValueOperand()->getType();
      } else {
        continue;
      }

      if (!Dep.MarkedValues.count(Ptr))
        continue;

      Value *Base = traceToBase(Ptr);
      if (!Base)
        continue;

      if (auto *Alloca = dyn_cast<AllocaInst>(Base)) {
        auto *ArraySize = dyn_cast<ConstantInt>(Alloca->getArraySize());
        if (!ArraySize)
          continue;
        Type *Form = Alloca->getAllocatedType();
        uint64_t ElemSize = DL.getTypeAllocSize(Form).getFixedValue();
        uint64_t TotalSize = ElemSize * ArraySize->getZExtValue();
        Res.Objects[&I] = {Alloca,    &F,  0, static_cast<int64_t>(TotalSize),
                           TotalSize, Form};
      } else {
        // Pointer-typed argument: extent is unknown, use the access type.
        uint64_t Size = DL.getTypeAllocSize(AccessType).getFixedValue();
        Res.Objects[&I] = {Base, &F,        0, static_cast<int64_t>(Size),
                           Size, AccessType};
      }
    }
  }

  return Res;
}

PreservedAnalyses DataPrinterPass::run(Function &F,
                                       FunctionAnalysisManager &FAM) {
  auto &Res = FAM.getResult<DataAnalysis>(F);

  // Sort by position in IR for deterministic output.
  std::vector<std::pair<Instruction *, MemoryObject *>> Sorted;
  for (auto &[I, Obj] : Res.Objects)
    Sorted.push_back({I, &Obj});

  std::sort(Sorted.begin(), Sorted.end(), [&F](auto &A, auto &B) {
    for (auto &BB : F)
      for (auto &I : BB) {
        if (&I == A.first)
          return true;
        if (&I == B.first)
          return false;
      }
    return false;
  });

  outs() << "Data objects for " << F.getName() << ":\n";
  for (auto &[I, Obj] : Sorted) {
    outs() << "  " << *I << "\n"
           << "    base:     " << *Obj->Base << "\n"
           << "    form:     " << *Obj->Form << "\n"
           << "    size:     " << Obj->Size << " bytes\n"
           << "    interval: [" << Obj->OffsetLow << ", " << Obj->OffsetHigh
           << ")\n";
  }

  return PreservedAnalyses::all();
}

} // namespace ct
