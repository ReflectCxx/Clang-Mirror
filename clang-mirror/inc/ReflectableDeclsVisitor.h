#pragma once

#include <map>
#include "clang/AST/RecursiveASTVisitor.h"

namespace clmirror {
    class RtlCodeGenerator;
}

namespace clmirror {

    class ReflectableDeclsVisitor : public clang::RecursiveASTVisitor<ReflectableDeclsVisitor>
    {
        RtlCodeGenerator& m_codegen;

    public:

        ReflectableDeclsVisitor(const std::string& pCurrentSrcFile);

        bool VisitFunctionDecl(clang::FunctionDecl* pFuncDecl);
    };
}