
#include <iostream>
#include <unordered_set>

#include "ClangReflectActionFactory.h"
#include "ReflectableDeclsVisitor.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"
#include "ASTParser.h"
#include "Logger.h"
#include "clang/StaticAnalyzer/Frontend/AnalysisConsumer.h"

namespace {

	class FindRecordDeclsConsumer : public clang::ASTConsumer
	{
		const std::string& m_currentSrcFile;

	public:

		FindRecordDeclsConsumer(const std::string& pSrcFile)
			: m_currentSrcFile(pSrcFile)
		{ }

		virtual void HandleTranslationUnit(clang::ASTContext& Context)
		{
			clmirror::ReflectableDeclsVisitor visitor(m_currentSrcFile);
			visitor.TraverseDecl(Context.getTranslationUnitDecl());
		}
	};


	class FindRecordDeclsAction : public clang::ASTFrontendAction
	{
		std::string m_targetSrcFile;

	public:

		FindRecordDeclsAction() = default;

		std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance& Compiler, llvm::StringRef InFile) override
		{
//			/*insignificant LOC. 
//			  just to force linker to link with dependent libs.*/
//#if !defined(_WIN32) && !defined(_WIN64)
//			clang::ento::CreateAnalysisConsumer(Compiler);
//#endif		//--ends--!

			Compiler.getDiagnosticOpts().ShowCarets = false;
			return std::make_unique<FindRecordDeclsConsumer>(m_targetSrcFile);
		}

		bool BeginSourceFileAction(clang::CompilerInstance& CI) override {
			const auto& inputs = CI.getInvocation().getFrontendOpts().Inputs;
			m_targetSrcFile = inputs[0].getFile().str();
			return true;
		}
	};
}


namespace clmirror {

	std::unique_ptr<clang::FrontendAction> CLMirrorActionFactory::create()
	{
		return std::make_unique<FindRecordDeclsAction>();
	}
}