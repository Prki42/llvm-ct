#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"

#include "CTDataPass.h"
#include "DataAnalysis.h"

#include <vector>

using namespace llvm;

namespace ct {

// Replaces a sensitive load with a stride over all candidate slots.
// Every slot is unconditionally loaded; a chain of selects picks the
// value from the slot whose pointer matches the original pointer.
static Value *emitCTLoad(LoadInst *LI, const MemoryObject &Obj,
                         const DataLayout &DL, IRBuilder<> &B) {
  Type *T = LI->getType();
  Value *OrigPtr = LI->getPointerOperand();
  LLVMContext &Ctx = T->getContext();

  uint64_t Stride = DL.getTypeAllocSize(T).getFixedValue();
  uint64_t N = static_cast<uint64_t>(Obj.OffsetHigh - Obj.OffsetLow) / Stride;

  Value *Result = UndefValue::get(T);
  for (uint64_t i = 0; i < N; i++) {
    uint64_t ElemIdx = static_cast<uint64_t>(Obj.OffsetLow) / Stride + i;
    Value *SlotPtr = B.CreateGEP(T, Obj.Base,
                                 ConstantInt::get(Type::getInt64Ty(Ctx), ElemIdx));
    Value *Loaded = B.CreateLoad(T, SlotPtr);
    Value *Cmp = B.CreateICmpEQ(OrigPtr, SlotPtr);
    Result = B.CreateSelect(Cmp, Loaded, Result);
  }
  return Result;
}

// Replaces a sensitive store with a stride over all candidate slots.
// Every slot is unconditionally read and written back; only the matching
// slot receives the new value, all others are written with their current value.
static void emitCTStore(StoreInst *SI, const MemoryObject &Obj,
                        const DataLayout &DL, IRBuilder<> &B) {
  Value *Val = SI->getValueOperand();
  Type *T = Val->getType();
  Value *OrigPtr = SI->getPointerOperand();
  LLVMContext &Ctx = T->getContext();

  uint64_t Stride = DL.getTypeAllocSize(T).getFixedValue();
  uint64_t N = static_cast<uint64_t>(Obj.OffsetHigh - Obj.OffsetLow) / Stride;

  for (uint64_t i = 0; i < N; i++) {
    uint64_t ElemIdx = static_cast<uint64_t>(Obj.OffsetLow) / Stride + i;
    Value *SlotPtr = B.CreateGEP(T, Obj.Base,
                                 ConstantInt::get(Type::getInt64Ty(Ctx), ElemIdx));
    Value *Existing = B.CreateLoad(T, SlotPtr);
    Value *Cmp = B.CreateICmpEQ(OrigPtr, SlotPtr);
    Value *Sel = B.CreateSelect(Cmp, Val, Existing);
    B.CreateStore(Sel, SlotPtr);
  }
}

PreservedAnalyses CTDataPass::run(Function &F, FunctionAnalysisManager &FAM) {
  auto &DataRes = FAM.getResult<DataAnalysis>(F);

  if (DataRes.Objects.empty())
    return PreservedAnalyses::all();

  const DataLayout &DL = F.getParent()->getDataLayout();

  // Collect upfront to avoid iterator invalidation during transformation.
  std::vector<Instruction *> Worklist;
  for (auto &[I, _] : DataRes.Objects)
    Worklist.push_back(I);

  bool Changed = false;
  for (auto *I : Worklist) {
    auto It = DataRes.Objects.find(I);
    if (It == DataRes.Objects.end())
      continue;

    const MemoryObject &Obj = It->second;

    // Argument-based objects have unknown extent — skip for now.
    if (isa<Argument>(Obj.Base))
      continue;

    IRBuilder<> B(I);

    if (auto *LI = dyn_cast<LoadInst>(I)) {
      Value *Result = emitCTLoad(LI, Obj, DL, B);
      LI->replaceAllUsesWith(Result);
      LI->eraseFromParent();
      Changed = true;
    } else if (auto *SI = dyn_cast<StoreInst>(I)) {
      emitCTStore(SI, Obj, DL, B);
      SI->eraseFromParent();
      Changed = true;
    }
  }

  return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

} // namespace ct
