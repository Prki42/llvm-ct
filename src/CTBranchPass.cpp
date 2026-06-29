#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/RegionInfo.h"
#include "llvm/IR/Analysis.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

#include "ArgDepAnalysis.h"
#include "CTBranchPass.h"

#include <cassert>
using namespace llvm;

#define DEBUG_TYPE "ct-branch-linearization"
#include "Debug.h"

namespace ct {

static void collectReachable(BasicBlock *Start, BasicBlock *Boundary,
                             SmallPtrSetImpl<BasicBlock *> &Reachable) {
  SmallVector<BasicBlock *, 16> Worklist;
  if (Start != Boundary && Reachable.insert(Start).second)
    Worklist.push_back(Start);

  while (!Worklist.empty()) {
    auto *BB = Worklist.pop_back_val();
    for (auto *Succ : successors(BB)) {
      if (Succ != Boundary && Reachable.insert(Succ).second)
        Worklist.push_back(Succ);
    }
  }
}

static bool hasSharedBlocks(const SmallPtrSetImpl<BasicBlock *> &A,
                            const SmallPtrSetImpl<BasicBlock *> &B) {
  for (auto *BB : A)
    if (B.count(BB))
      return true;
  return false;
}

static BasicBlock *findSingleExitBlock(const SmallPtrSetImpl<BasicBlock *> &Set,
                                       BasicBlock *IPDOM) {
  BasicBlock *Exit = nullptr;
  for (auto *BB : Set) {
    for (auto *Succ : successors(BB)) {
      if (Succ == IPDOM) {
        if (Exit && Exit != BB)
          llvm_unreachable("Multiple exit blocks");
        Exit = BB;
        break;
      }
    }
  }
  return Exit;
}

static bool containsCall(const SmallPtrSetImpl<BasicBlock *> &Blocks) {
  for (auto *BB : Blocks)
    for (auto &I : *BB)
      if (isa<CallInst>(&I))
        return true;
  return false;
}

static void pullDownCondToLoops(SmallPtrSetImpl<BasicBlock *> &Blocks,
                                LoopInfo &LI, Value *Cond) {
  for (auto *BB : Blocks) {
    auto *L = LI.getLoopFor(BB);
    if (!L || !L->isLoopExiting(BB))
      continue;

    auto *BI = dyn_cast<BranchInst>(BB->getTerminator());
    if (!BI || !BI->isConditional())
      continue;

    // Figure out which successor stays in the loop
    BasicBlock *ExitSucc = nullptr;
    for (unsigned i = 0; i < 2; i++) {
      if (!L->contains(BI->getSuccessor(i))) {
        ExitSucc = BI->getSuccessor(i);
        break;
      }
    }
    if (!ExitSucc)
      continue;

    IRBuilder<> B(BI);
    if (BI->getSuccessor(0) == ExitSucc) {
      // true -> exit: OR with !Cond to force exit
      Value *NotCond = B.CreateNot(Cond);
      BI->setCondition(B.CreateOr(BI->getCondition(), NotCond));
    } else {
      // false -> exit: AND with Cond to force exit
      BI->setCondition(B.CreateAnd(BI->getCondition(), Cond));
    }
  }
}

static void convertIPDOMPhis(BasicBlock *IPDOM, BasicBlock *TrueValBlock,
                             BasicBlock *FalseValBlock,
                             BasicBlock *RemovedBlock, Value *Cond) {
  BasicBlock *RemainingBlock =
      (TrueValBlock == RemovedBlock) ? FalseValBlock : TrueValBlock;

  // Since it could be possible for IPDOM to also have other
  // predecessors - we're inserting select to the end of the
  // block that will jump to IPDOM and changing IPDOM's phis
  // to use the the new select for the incoming value.

  IRBuilder<> B(RemainingBlock->getTerminator());

  for (auto &Phi : make_early_inc_range(IPDOM->phis())) {
    // Only process phis that have incoming from both blocks
    int TrueIdx = Phi.getBasicBlockIndex(TrueValBlock);
    int FalseIdx = Phi.getBasicBlockIndex(FalseValBlock);
    if (TrueIdx < 0 || FalseIdx < 0)
      llvm_unreachable("IPDOM phi missing expected incoming blocks");

    Value *TrueVal = Phi.getIncomingValue(static_cast<unsigned int>(TrueIdx));
    Value *FalseVal = Phi.getIncomingValue(static_cast<unsigned int>(FalseIdx));

    Value *Sel = B.CreateSelect(Cond, TrueVal, FalseVal);

    Phi.removeIncomingValue(RemovedBlock, false);
    Phi.setIncomingValueForBlock(RemainingBlock, Sel);

    // If phi now has exactly one incoming, replace it entirely
    // (phi didn't have any other incoming branches)
    if (Phi.getNumIncomingValues() == 1) {
      Phi.replaceAllUsesWith(Sel);
      Phi.eraseFromParent();
    }
  }
}

// For every store in the given blocks, replace:
//   store val, ptr
// with:
//   existing = load T, ptr
//   sel      = select(Cond, val, existing)   [if CondIsTrue]
//            = select(Cond, existing, val)   [if !CondIsTrue]
//   store sel, ptr
//
// This makes stores inside a linearized branch semantically conditional:
// the write is a no-op on the path that the original branch would not take.
static void guardStores(const SmallPtrSetImpl<BasicBlock *> &Blocks,
                        Value *Cond, bool CondIsTrue) {
  SmallVector<StoreInst *, 16> Stores;
  for (auto *BB : Blocks)
    for (auto &I : *BB)
      if (auto *SI = dyn_cast<StoreInst>(&I))
        Stores.push_back(SI);

  for (auto *SI : Stores) {
    IRBuilder<> B(SI);
    Value *Val = SI->getValueOperand();
    Value *Ptr = SI->getPointerOperand();
    Value *Existing = B.CreateLoad(Val->getType(), Ptr);
    Value *Sel = CondIsTrue ? B.CreateSelect(Cond, Val, Existing)
                            : B.CreateSelect(Cond, Existing, Val);
    SI->setOperand(0, Sel);
  }
}

static bool tryLinearize(BranchInst *BI, PostDominatorTree &PDT,
                         DominatorTree &DT, LoopInfo &LI, RegionInfo &RI) {
  BasicBlock *EntryBB = BI->getParent();
  Value *Cond = BI->getCondition();
  BasicBlock *ThenBB = BI->getSuccessor(0);
  BasicBlock *ElseBB = BI->getSuccessor(1);

  // Not necessary - early leave if dead code
  if (!DT.isReachableFromEntry(EntryBB)) {
    LLVM_DEBUG(dbgs() << "Skipping (dead code)\n");
    return false;
  }

  // Not necessary - early leave
  // (since we're checking with containsLoop later)
  if (auto *L = LI.getLoopFor(EntryBB))
    if (L->isLoopExiting(EntryBB))
      return false;

  // maybe change to assertion?
  auto *Node = PDT[EntryBB];
  if (!Node || !Node->getIDom())
    return false;
  BasicBlock *IPDOM = Node->getIDom()->getBlock();

  // Get region information for the EntryBB
  // We assume the pass has been called after 'structurizecfg'
  //
  // However, resulting assumptions should be challenged with assertions
  // for testing purposes
  Region *R = RI.getRegionFor(EntryBB);
  assert(R && "EntryBB not in any region");

  if (R->getEntry() != EntryBB || R->getExit() != IPDOM) {
    LLVM_DEBUG(dbgs() << "  Skipping (not SESE)\n");
    return false;
  }

  // TODO maybe add additional assertions that check for outside branches

  bool ThenEmpty = (ThenBB == IPDOM);
  bool ElseEmpty = (ElseBB == IPDOM);
  if (ThenEmpty && ElseEmpty)
    return false;

  // In case both then and else are != IPDOM rewiring will
  // change predecessor of one of them and possibly invalidate
  // some phi instruction.
  // (structurizecfg should make this impossible)
  if (!ThenEmpty)
    assert(ThenBB->phis().empty() &&
           "nonempty then branch has phi instructions");
  if (!ElseEmpty)
    assert(ElseBB->phis().empty() &&
           "nonempty else branch has phi instructions");

  SmallPtrSet<BasicBlock *, 16> TrueReachable, FalseReachable;
  collectReachable(ThenBB, IPDOM, TrueReachable);
  collectReachable(ElseBB, IPDOM, FalseReachable);

  assert(!hasSharedBlocks(TrueReachable, FalseReachable) &&
         "should be ran with structurizecfg which splits shared blocks");

  if (containsCall(TrueReachable) || containsCall(FalseReachable))
    return false;

  // Linearizing branches that contain loops can change semantics of the inner
  // loop. (if exit condition is connected to the if condition)
  //
  // Instead of skiping, we pull down the if condition to the loop
  // and still have correct semantics.
  IRBuilder<> B(BI);
  auto *NotCond = B.CreateNot(Cond);
  pullDownCondToLoops(TrueReachable, LI, Cond);
  pullDownCondToLoops(FalseReachable, LI, NotCond);

  BasicBlock *TrueExitBB =
      ThenEmpty ? nullptr : findSingleExitBlock(TrueReachable, IPDOM);
  BasicBlock *FalseExitBB =
      ElseEmpty ? nullptr : findSingleExitBlock(FalseReachable, IPDOM);

  assert((ThenEmpty || TrueExitBB) && "structurizecfg guarantees single exit");
  assert((ElseEmpty || FalseExitBB) && "structurizecfg guarantees single exit");

  LLVM_DEBUG(dbgs() << "  Linearizing: " << *BI << "\n"
                    << "    IPDOM: " << IPDOM->getName() << "\n"
                    << "    ThenBB: " << ThenBB->getName()
                    << (ThenEmpty ? " (empty)" : "") << "\n"
                    << "    ElseBB: " << ElseBB->getName()
                    << (ElseEmpty ? " (empty)" : "") << "\n");

  // Convert IPDOM phis to selects
  //
  // Both non-empty:
  //   TrueExitBB stays
  //   -> IPDOM, FalseExitBB gets redirected
  //   -> ThenBB
  // ElseEmpty:
  //   TrueExitBB stays
  //    -> IPDOM, EntryBB gets redirected
  //    -> ThenBB
  // ThenEmpty:
  //   FalseExitBB stays
  //   -> IPDOM, EntryBB gets redirected
  //   -> ElseBB

  BasicBlock *TrueValBlock, *FalseValBlock, *RemovedBlock;

  if (ThenEmpty) {
    TrueValBlock = EntryBB;
    FalseValBlock = FalseExitBB;
    RemovedBlock = EntryBB;
  } else if (ElseEmpty) {
    TrueValBlock = TrueExitBB;
    FalseValBlock = EntryBB;
    RemovedBlock = EntryBB;
  } else {
    TrueValBlock = TrueExitBB;
    FalseValBlock = FalseExitBB;
    RemovedBlock = FalseExitBB;
  }

  convertIPDOMPhis(IPDOM, TrueValBlock, FalseValBlock, RemovedBlock, Cond);

  // Guard stores inside linearized blocks so memory writes remain conditional.
  // Stores in TrueReachable only take effect when Cond==true;
  // stores in FalseReachable only take effect when Cond==false.
  guardStores(TrueReachable, Cond, true);
  guardStores(FalseReachable, Cond, false);

  // Rewire branches
  //   entry -> else side -> then side -> IPDOM

  if (ThenEmpty) {
    // Only false side: entry -> ElseBB -> ... -> IPDOM
    BI->eraseFromParent();
    BranchInst::Create(ElseBB, EntryBB);
  } else if (ElseEmpty) {
    // Only true side: entry -> ThenBB -> ... -> IPDOM
    BI->eraseFromParent();
    BranchInst::Create(ThenBB, EntryBB);
  } else {
    // Both sides: entry -> ElseBB -> ... -> FalseExitBB -> ThenBB -> ... ->
    // IPDOM
    BI->eraseFromParent();
    BranchInst::Create(ElseBB, EntryBB);

    // Redirect false side's exit from IPDOM to ThenBB
    auto *FalseExitTerm = FalseExitBB->getTerminator();
    for (unsigned i = 0; i < FalseExitTerm->getNumSuccessors(); i++) {
      if (FalseExitTerm->getSuccessor(i) == IPDOM)
        FalseExitTerm->setSuccessor(i, ThenBB);
    }
  }

  LLVM_DEBUG(dbgs() << "  Done.\n");
  return true;
}

PreservedAnalyses CTBranchPass::run(Function &F, FunctionAnalysisManager &FAM) {
  bool EverChanged = false;

  LLVM_DEBUG(dbgs() << "\n[CTBranchPass] Processing " << F.getName() << "\n");

  // Since we're changing CFG on every try_linearize success
  // we could potentially invalidate some previous analysis.
  //
  // There almost certainly are ways to avoid this but we don't
  // have the luxury to care.
  while (true) {
    auto &Dep = FAM.getResult<ArgDepAnalysis>(F);
    auto &PDT = FAM.getResult<PostDominatorTreeAnalysis>(F);
    auto &DT = FAM.getResult<DominatorTreeAnalysis>(F);
    auto &LI = FAM.getResult<LoopAnalysis>(F);
    auto &RI = FAM.getResult<RegionInfoAnalysis>(F);

    bool Found = false;
    for (auto *BI : Dep.MarkedBranches) {
      if (tryLinearize(BI, PDT, DT, LI, RI)) {
        Found = true;
        break;
      }
    }

    if (!Found)
      break;

    EverChanged = true;
    FAM.invalidate(F, PreservedAnalyses::none());

    LLVM_DEBUG(dbgs() << "  After linearization:\n" << F << "\n");
  }

  return EverChanged ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

} // namespace ct
