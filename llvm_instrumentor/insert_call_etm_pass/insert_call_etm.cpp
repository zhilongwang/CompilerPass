#include "llvm/Pass.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/CallGraphSCCPass.h"
#include "llvm/Support/Casting.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include <utility>
#define DEBUG_TYPE "mba" 
#include "llvm/Support/Debug.h"
#include <iostream>
#include <list>
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
using namespace llvm;

namespace {
  struct insert_call_etm : public ModulePass {

    static char ID;
    insert_call_etm() : ModulePass(ID) {}

    bool runOnModule(Module &M) override;
    
    bool doInitialization(Module &M) override;
  };
}
bool insert_call_etm::runOnModule(Module &M)
{
	/*
	**Get reference to SYS_etm(uint) from the library
	*/
	Type *I32Ty = Type::getInt32Ty(M.getContext());	
	auto& context		=M.getContext();
	auto* helpTy		=FunctionType::get(Type::getVoidTy(context),Type::getInt16Ty(context),false);
	Function *SYS_etm	= cast<Function>(M.getOrInsertFunction("SYS_etm", helpTy));
	
	for (auto &F : M){
		for(auto &BB : F){
			/*
			**Get ins num of a basicblock
			*/
			short num_atomic_inst = 0;
  			for (auto it = BB.begin(); it != BB.end(); it++) {
      				num_atomic_inst++;
			}
			if(num_atomic_inst>0){
				/*Returns an iterator to the first instruction
				** in this block that is suitable for inserting
				** a non-PHI instruction.	
				*/
				BasicBlock::iterator IP	=BB.getFirstInsertionPt();
				IRBuilder<> IRB(&(*IP));
				/*Insert function call*/
				auto *constint = IRB.getInt16(num_atomic_inst++);
				IRB.CreateCall(SYS_etm,constint );
			}
		}
	}
	return true;
}

bool insert_call_etm::doInitialization(Module &M)
{
	return true;
}

char insert_call_etm::ID = 0;

// Registration to run by default using clang compiler
static void registerMyPass(const PassManagerBuilder &, legacy::PassManagerBase &PM)
{
  PM.add(new insert_call_etm());
}

static RegisterStandardPasses RegisterMyPass1(PassManagerBuilder::EP_ModuleOptimizerEarly, registerMyPass);
static RegisterStandardPasses RegisterMyPass2(PassManagerBuilder::EP_EnabledOnOptLevel0, registerMyPass);
