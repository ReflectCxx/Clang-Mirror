
#include <fstream>
#include <iostream>
#include <filesystem>

#include "Constants.h"
#include "Logger.h"
#include "ClangDriver.h"
#include "RtlCodeManager.h"

int main(int argc, const char** argv)
{
    const auto& tsBegin = clmirror::Clock::now();

    if (clmirror::ClangDriver::compileSourceFiles(argc, argv))
    {
        clmirror::RtlCodeManager::Instance().generateRegistrationCode();
    }
    else 
    {
        clmirror::Logger::outException("\n[clang-mirror failed]");
    }

    std::cout << clmirror::RESET << std::flush;
    const auto& tsEnd = std::chrono::duration_cast<clmirror::Second> (clmirror::Clock::now() - tsBegin).count();
    clmirror::Logger::out("Total time elapsed: " + std::to_string(tsEnd) + "\n");

    return 0;
}