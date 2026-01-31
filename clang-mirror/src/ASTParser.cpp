
#include "ASTParser.h"
#include "ASTParserUtils.h"

#include <mutex>
#include <iostream>
#include <filesystem>

#include "Logger.h"
#include "Constants.h"
#include "ClangReflectDiagnosticConsumer.h"
#include "ClangReflectActionFactory.h"

using namespace llvm;
using namespace clang;
using namespace clang::tidy;
using namespace clang::tooling;

namespace clmirror
{
	ASTParser::ASTParser(const std::vector<std::string>& pFiles,
					     clang::tooling::CompilationDatabase& pCdb)
		: m_files(pFiles)
		, m_compileDb(pCdb)
	{ }


	const int ASTParser::parseFiles(const int pStartIndex, const int pEndIndex)
	{
		for (size_t index = pStartIndex; index <= pEndIndex; index++)
		{
			const auto& srcFilePath = m_files.at(index).c_str();

			Logger::outProgress("compiling: " + std::string(srcFilePath));

			if (!std::filesystem::exists(srcFilePath)) {
				Logger::outProgress(srcFilePath + std::string(". File not found..!"), false);
				continue;
			}

			llvm::IntrusiveRefCntPtr<vfs::OverlayFileSystem> baseFS = createBaseFS();
			if (!baseFS) {
				Logger::out("Failed to initialize vfs::OverlayFileSystem.");
				return false;
			}

			ClangTool clangTool(m_compileDb, { srcFilePath }, std::make_shared<PCHContainerOperations>(), baseFS);

			auto OwningOptionsProvider = createOptionsProvider(baseFS);

			ClangTidyContext context(std::move(OwningOptionsProvider), false, false);
			context.setEnableProfiling(false);

			ClangReflectDiagnosticConsumer diagConsumer(context);
			auto diagOpts = std::make_unique<DiagnosticOptions>();
			DiagnosticsEngine diagEngine(new DiagnosticIDs(), *diagOpts, &diagConsumer, false);
			
			context.setDiagnosticsEngine(std::move(diagOpts), &diagEngine);
			clangTool.setDiagnosticConsumer(&diagConsumer);

			auto actionFactory = std::make_unique<CLMirrorActionFactory>();
			clangTool.run(actionFactory.get());

			auto missingHeaderErrors = diagConsumer.getMissingHeaderMsgs();
			Logger::outReflectError(srcFilePath, std::vector<std::string>(), missingHeaderErrors);
		}
		return 0;
	}
}