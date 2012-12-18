//
//  main.cpp
//  silk
//
//  Created by Haohui Mai on 9/19/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//
//

#include "silk/decil/IHost.h"
#include "silk/VMCore/VMModel.h"
//#include "silk/Target/TargetInfo.h"
//#include "silk/Support/ErrorHandler.h"
//
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
#include <llvm/PassManager.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/ADT/OwningPtr.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/PrettyStackTrace.h>
#include <llvm/Support/Signals.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Support/system_error.h>

using namespace llvm;
using namespace silk;

static cl::list<std::string>
ClassPaths("classpath", cl::desc("<class path>"));

static cl::opt<std::string>
TargetTriple("target-triple", cl::desc("<target description>"));

static cl::opt<std::string>
InputFilename(cl::Positional, cl::desc("<input assembly>"), cl::init("-"));

static cl::opt<std::string>
OutputFilename("o", cl::desc("Output filename"), cl::value_desc("filename"));

static cl::opt<bool>
DisableVerify("disable-verify", cl::desc("Do not run verify pass"), cl::init(false));

namespace silk
{
    Pass *CreateRuntimeHelperFixupPass(IIntrinsic *intrinsic);
}

int main(int argc, const char * argv[])
{
    sys::PrintStackTraceOnErrorSignal();
    PrettyStackTraceProgram X(argc, argv);
    cl::ParseCommandLineOptions(argc, argv, "MSIL AOT compiler\n");
    
    llvm_shutdown_obj Y;  // Call llvm_shutdown() on exit.
    OwningPtr<tool_output_file> Out;
    std::string ErrorInfo;
    Out.reset(new tool_output_file(OutputFilename.c_str(), ErrorInfo,
                                   raw_fd_ostream::F_Binary));
    if (!ErrorInfo.empty())
    {
        errs() << ErrorInfo << '\n';
        return 1;
    }
//
//    ErrorHandler eh;
//
//    Module m("", getGlobalContext());
//    m.setModuleIdentifier(InputFilename);
//    
//    TargetInfo *ti = TargetInfo::Create(TargetDescription, &m);
//
//    Host host(&m, ti, &eh);
    
    decil::IHost *host = decil::CreateDefaultHost();
    for (size_t i = 0; i < ClassPaths.size(); ++i)
        host->AddClassPath(ClassPaths[i]);
    
    host->LoadAssembly(InputFilename);
    
    if (host->error_handler().has_error())
    {
        host->error_handler().Error("Failed to load assembly `%s`.", InputFilename.c_str());
        return 1;
    }
    
    auto compilation_engine = CreateCompilationEngine(host, TargetTriple);
    auto aot_intrinsic = CreateAOTIntrinsic(compilation_engine->module());
    compilation_engine->set_intrinsic(aot_intrinsic);
    compilation_engine->Compile();
    
    auto m = compilation_engine->module();
    PassManager Passes;
    //
    //    if (!DisableVerify)
    Passes.add(createVerifierPass());
    Passes.add(CreateRuntimeHelperFixupPass(aot_intrinsic));
    Passes.add(createBitcodeWriterPass(Out->os()));

    Passes.run(*m);
    Out->keep();
    
    return 0;
}
