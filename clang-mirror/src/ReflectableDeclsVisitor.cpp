
#include <iostream>
#include <algorithm>

#include "Constants.h"
#include "StringUtils.h"
#include "RtlCodeManager.h"
#include "RtlCodeGenerator.h"
#include "ReflectableDeclsUtils.h"
#include "ReflectableDeclsVisitor.h"

using namespace clang;

namespace clmirror
{
	ReflectableDeclsVisitor::ReflectableDeclsVisitor(const std::string& pCurrentSrcFile)
		: m_codegen(RtlCodeManager::Instance().initCodeGenerator(pCurrentSrcFile))
	{ }


	bool ReflectableDeclsVisitor::VisitFunctionDecl(FunctionDecl* pFnDecl)
	{
		if (!ReflectableDeclsUtils::isInUserCode(pFnDecl) ||
			pFnDecl->isDeleted() ||
			pFnDecl->isInAnonymousNamespace() ||
			(pFnDecl->isGlobal() && pFnDecl->isStatic()) ||
			pFnDecl->isOverloadedOperator() ||
			pFnDecl->getKind() == Decl::Kind::CXXDestructor ||
			pFnDecl->getAccess() == AS_private ||
			pFnDecl->getAccess() == AS_protected ||
			pFnDecl->getLinkageInternal() != Linkage::External) {
			return true;
		}

		if (!pFnDecl->isThisDeclarationADefinition()) {
			return true;
		}
		
		if (pFnDecl->getFirstDecl() == nullptr) {
			return true;
		}

		if (!ReflectableDeclsUtils::isDeclFrmCurrentSource(m_codegen.getSrcFile(), pFnDecl)) {
			return true;
		}

		std::string declSrcFile;
		auto& SM = pFnDecl->getASTContext().getSourceManager();
		for (auto* D : pFnDecl->redecls())
		{
			SourceLocation loc = SM.getSpellingLoc(D->getLocation());
			if (!loc.isValid()) {
				continue;
			}

			StringRef fileName = SM.getFilename(loc);
			if (fileName.ends_with(".h") || fileName.ends_with(".hpp"))
			{
				declSrcFile = fileName.str();
				break;
			}
		}

		if (!declSrcFile.empty()) 
		{
			std::vector<std::string> parmTypes;
			const auto& params = pFnDecl->parameters();
			const auto& fnQName = pFnDecl->getQualifiedNameAsString();
			for (unsigned index = 0; index < params.size(); index++)
			{
				if (params[index]->isInAnonymousNamespace()) {
					//m_unreflectedFunctions.push_back(fnQName);
					return true;
				}
				if (params[index]->isInvalidDecl()) {
					//m_unreflectedFunctions.push_back(fnQName);
					return true;
				}
				parmTypes.push_back(ReflectableDeclsUtils::extractParameterType(params[index]));
			}

			std::string functionName;
			MetaKind metaKind = MetaKind::None;

			if (llvm::isa<clang::CXXConstructorDecl>(pFnDecl)) 
			{
				metaKind = MetaKind::Ctor;
				functionName = pFnDecl->getDeclName().getAsString();
			}
			else if (const auto* method = llvm::dyn_cast<clang::CXXMethodDecl>(pFnDecl)) 
			{
				if (method->isStatic()) {
					metaKind = MetaKind::MemberFnStatic;
				}
				else {
					if (method->isConst()) {
						metaKind = MetaKind::MemberFnConst;
					}
					else {
						metaKind = MetaKind::MemberFnNonConst;
					}
				}
				functionName = pFnDecl->getDeclName().getAsString();
			}
			else {
				metaKind = MetaKind::NonMemberFn;
				functionName = pFnDecl->getQualifiedNameAsString();
			}
			const std::string recordStr = ReflectableDeclsUtils::extractParentTypeName(pFnDecl);
			RtlCodeManager::Instance().addFunctionSignature(metaKind, declSrcFile, recordStr, functionName, parmTypes);
		}
		return true;
	}
}