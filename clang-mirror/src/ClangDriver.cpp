
#include <set>
#include <map>
#include <deque>
#include <vector>
#include <string>
#include <thread>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "Logger.h"
#include "ASTParser.h"
#include "ClangDriver.h"

#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/TargetParser/Host.h"

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/CompilationDatabase.h"

#include "clang-tidy/ClangTidyOptions.h"
#include "clang-tidy/ClangTidyDiagnosticConsumer.h"

using namespace llvm;
using namespace clang::tidy;
using namespace clang::tooling;

namespace 
{
    static cl::OptionCategory ClangMirrorCategory("clang-mirror options");
}


namespace clmirror
{
    bool ClangDriver::compileSourceFiles(int argc, const char** argv)
    {
        InitLLVM X(argc, argv);
        SmallVector<const char*> Args{ argv, argv + argc };

        const bool isWin = Triple(sys::getProcessTriple()).isOSWindows();
        cl::TokenizerCallback Tokenizer = isWin ? cl::TokenizeWindowsCommandLine
                                                : cl::TokenizeGNUCommandLine;
        BumpPtrAllocator Alloc;
        cl::ExpansionContext ECtx(Alloc, Tokenizer);
        if (Error Err = ECtx.expandResponseFiles(Args)) {
            WithColor::error() << toString(std::move(Err)) << "\n";
            return false;
        }

        argc = static_cast<int>(Args.size());
        argv = Args.data();

        Expected<CommonOptionsParser> OptionsParser = 
            CommonOptionsParser::create(argc, argv, ClangMirrorCategory, cl::ZeroOrMore);

        if (!OptionsParser) {
            llvm::WithColor::error() << llvm::toString(OptionsParser.takeError());
            Logger::out("Failed to initialize CommonOptionsParser.");
            return false;
        }

        std::string cdbLoadErr;
        StringRef cdbPathStr("dummy");
        auto pathList = OptionsParser->getSourcePathList();
        if (!pathList.empty()) {
            cdbPathStr = pathList.front();
        }

        auto cdb = OptionsParser->getCompilations().loadFromDirectory(cdbPathStr, cdbLoadErr);
        if (cdb)
        {
            const auto& srcFiles = cdb->getAllFiles();
            Logger::out("Number of source files in CDB: " + std::to_string(srcFiles.size()));
            std::unordered_set<std::string> distinctSrcFiles(srcFiles.begin(), srcFiles.end());
            Logger::out("Number of distinct source files in CDB: " + std::to_string(distinctSrcFiles.size()));
            const auto& finalSrcFiles = std::vector<std::string>(distinctSrcFiles.begin(), distinctSrcFiles.end());
            runClangParser(finalSrcFiles, *cdb);
        }
        else
        {
            Logger::out("CDB not found at location : " + cdbPathStr.str());
            Logger::out("error : " + cdbLoadErr);
            return false;
        }
        return true;
    }


    void ClangDriver::runClangParser(const std::vector<std::string>& pSrcFiles, CompilationDatabase& pCdb)
    {
        const int fileCount = pSrcFiles.size();

        Logger::resetDoneCounter(fileCount);

        //TODO: get the number of threads from command line
        const int numCores = /*0; //*/std::thread::hardware_concurrency() - 2;
        const int numThreads = (numCores <= 0 ? 1 : numCores);

        int endIndex = 0;
        int startIndex = 0;
        int indexOffset = fileCount / numThreads;
        int restOffset = fileCount % numThreads;
        std::vector<std::thread> threadPool;

        while (endIndex < fileCount - 1)
        {
            endIndex = startIndex + (indexOffset + (--restOffset >= 0 ? 1 : 0)) - 1;
            endIndex = (endIndex > fileCount - 1) ? (fileCount - 1) : endIndex;

            auto thread = std::thread(
                [&](const int pStartIndex, const int pEndIndex) {

                    ASTParser cxxParser(pSrcFiles, pCdb);
                    cxxParser.parseFiles(pStartIndex, pEndIndex);
                },
                startIndex, endIndex);
            threadPool.push_back(std::move(thread));
            startIndex = endIndex + 1;
        }

        Logger::out("Running with number of threads: " + std::to_string(threadPool.size()));
        for (auto& thread : threadPool) {
            thread.join();
        }
    }
}
