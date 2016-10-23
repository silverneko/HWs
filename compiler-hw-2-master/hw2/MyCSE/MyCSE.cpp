#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include <vector>

using namespace llvm;

/*
 *  Perform local CSE
 */

namespace {
class MyCSE : public FunctionPass {
public:
  static char ID;
  MyCSE() : FunctionPass(ID) {}

  bool runOnFunction(Function&) override;
};

char MyCSE::ID(0);
static RegisterPass<MyCSE>
  X("myCSE", "Common Subexpression Elimination Pass", false, false);

using std::vector;

class LookupTable {
  vector<Instruction*> _store;
public:
  LookupTable() {}

  Instruction * lookup(Instruction * rhs) {
    for (Instruction * lhs : _store) {
      if (lhs->isIdenticalTo(rhs)) {
        return lhs;
      }
    }
    return nullptr;
  }

  void insert(Instruction * rhs) {
    _store.push_back(rhs);
  }

  void filterByAlloca(AllocaInst * inst) {
    int j = 0;
    for (int i = 0; i < _store.size(); ++i) {
      Instruction *it = _store[i];
      if (LoadInst * li = dyn_cast<LoadInst>(it)) {
        if (AllocaInst * ai = dyn_cast<AllocaInst>(li->getPointerOperand())) {
          if (ai != inst) {
            _store[j++] = it;
            continue;
          }
        }
      } else {
        _store[j++] = it;
      }
    }
    _store.resize(j);
  }

  void clearLoadInst() {
    int j = 0;
    for (int i = 0; i < _store.size(); ++i) {
      Instruction *it = _store[i];
      if (isa<LoadInst>(it)) {
        // do nothing
      } else {
        _store[j++] = it;
      }
    }
    _store.resize(j);
  }
};

bool isSimpleInstruction(const Instruction * inst) {
  if (inst->mayHaveSideEffects() || isa<TerminatorInst>(inst)) {
    return false;
  }
  return isa<BinaryOperator>(inst) || // Add, Sub, Mul ...
         isa<CmpInst>(inst) || // Fcmp or Icmp
         isa<GetElementPtrInst>(inst) ||  // pointer calculation inst
         isa<ExtractElementInst>(inst) || // extract elem from vector type
         isa<InsertElementInst>(inst) || // insert elem to vector type
         isa<ShuffleVectorInst>(inst) || // permutate vector type
         isa<SelectInst>(inst) || // select value by predicate
         isa<InsertValueInst>(inst) || // I don't exactly understand
         isa<ExtractValueInst>(inst) || // these two inst
         isa<CastInst>(inst) || // data casting inst
         isa<LoadInst>(inst); // load from address can be cached
}

bool MyCSE::runOnFunction(Function &F) {
  bool changed = false;
  int ec = 0;
  for (auto b = F.begin(); b != F.end(); ) {
    BasicBlock &BB = *b++;
    LookupTable table;
    for (auto i = BB.begin(); i != BB.end(); ) {
      Instruction *inst = i++;
      if (StoreInst * it = dyn_cast<StoreInst>(inst)) {
        Value * ptr = it->getPointerOperand();
        if (AllocaInst *localPtr = dyn_cast<AllocaInst>(ptr)) {
          table.filterByAlloca(localPtr);
        } else {
          table.clearLoadInst();
        }
        continue;
      }
      if (isSimpleInstruction(inst)) {
        if (Instruction *rhs = table.lookup(inst)) {
          //errs() << "erase"; inst->dump();
          inst->replaceAllUsesWith(rhs);
          inst->eraseFromParent();
          ++ec;
          changed = true;
        } else {
          table.insert(inst);
        }
      }
    }
  }
  errs() << "CSE count: " << ec << "\n";
  return changed;
}

}

