#include "ASTParser.h"

#include <mutex>
#include <iostream>
#include <filesystem>

#include "Logger.h"
#include "Constants.h"
#include "clang-tidy/ClangTidyDiagnosticConsumer.h"
#include "ClangReflectDiagnosticConsumer.h"
#include "ClangReflectActionFactory.h"

using namespace llvm;
using namespace clang;
using namespace clang::tidy;
using namespace clang::tooling;

namespace
{
	std::mutex g_mutex;
	static cl::OptionCategory toolCategory(clmirror::CLANG_MIRROR);
}



namespace
{
	static cl::desc desc(StringRef Description) { return { Description.ltrim() }; }

	static cl::OptionCategory ClangMirrorCategory("clang-mirror options");

	static cl::opt<std::string> VfsOverlay("vfsoverlay",
                                           desc(R"(Overlay the virtual filesystem described by file over the real file system.)"),
                                           cl::value_desc("filename"),
                                           cl::cat(ClangMirrorCategory));

	static std::unique_ptr<ClangTidyOptionsProvider> createOptionsProvider(llvm::IntrusiveRefCntPtr<vfs::FileSystem> FS)
	{
		ClangTidyOptions DefaultOptions;
		ClangTidyOptions OverrideOptions;
		ClangTidyGlobalOptions GlobalOptions;

		return std::make_unique<FileOptionsProvider>(std::move(GlobalOptions), std::move(DefaultOptions),
			                                         std::move(OverrideOptions), std::move(FS));
	}

	static llvm::IntrusiveRefCntPtr<vfs::FileSystem> getVfsFromFile(const std::string& OverlayFile,
                                                                    llvm::IntrusiveRefCntPtr<vfs::FileSystem> BaseFS)
	{
		llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> Buffer = BaseFS->getBufferForFile(OverlayFile);

		if (!Buffer) {
			llvm::errs() << "Can't load virtual filesystem overlay file '"
				<< OverlayFile << "': " << Buffer.getError().message()
				<< ".\n";
			return nullptr;
		}

		IntrusiveRefCntPtr<vfs::FileSystem> FS = vfs::getVFSFromYAML(std::move(Buffer.get()), /*DiagHandler*/ nullptr, OverlayFile);
		if (!FS) {
			llvm::errs() << "Error: invalid virtual filesystem overlay file '" << OverlayFile << "'.\n";
			return nullptr;
		}
		return FS;
	}

	static llvm::IntrusiveRefCntPtr<vfs::OverlayFileSystem> createBaseFS()
	{
		llvm::IntrusiveRefCntPtr<vfs::OverlayFileSystem> BaseFS(new vfs::OverlayFileSystem(vfs::getRealFileSystem()));
		if (!VfsOverlay.empty())
		{
			IntrusiveRefCntPtr<vfs::FileSystem> VfsFromFile = getVfsFromFile(VfsOverlay, BaseFS);
			if (!VfsFromFile) {
				return nullptr;
			}
			BaseFS->pushOverlay(std::move(VfsFromFile));
		}
		return BaseFS;
	}
}



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

			auto actionFactory = std::unique_ptr<ActionFactory>(new ActionFactory(context));
			clangTool.run(actionFactory.get());

			auto unreflectedFuncs = actionFactory->getUnreflectedFunctions();
			auto missingHeaderErrors = diagConsumer.getMissingHeaderMsgs();
			Logger::outReflectError(srcFilePath, unreflectedFuncs, missingHeaderErrors);
		}
		return 0;
	}
}