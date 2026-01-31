#pragma once

#include <memory>
#include "clang/Tooling/Tooling.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang-tidy/ClangTidyDiagnosticConsumer.h"

namespace clmirror 
{
	class CLMirrorActionFactory : public clang::tooling::FrontendActionFactory
	{
	public:

		std::unique_ptr<clang::FrontendAction> create() override;
	};
}