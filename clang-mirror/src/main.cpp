
#include <fstream>
#include <iostream>
#include <filesystem>

#include "Constants.h"
#include "Logger.h"
#include "ClangDriver.h"
#include "ReflectableInterface.h"

int main(int argc, const char** argv)
{
    const auto& tsBegin = clmirror::Clock::now();
    if (clmirror::ClangDriver::compileSourceFiles(argc, argv))
    {
        clmirror::ReflectableInterface::Instance().dump();
        const auto& tsEnd = std::chrono::duration_cast<clmirror::Second> (clmirror::Clock::now() - tsBegin).count();
        clmirror::Logger::out("Total time elapsed: " + std::to_string(tsEnd) + "\n");
    }
    else 
    {
        clmirror::Logger::outException("\n[clang-mirror failed]");
    }
    std::cout << clmirror::RESET << std::endl;
    return 0;
}