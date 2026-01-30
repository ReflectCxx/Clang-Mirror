
#include <iostream>
#include <algorithm>

#include "Constants.h"
#include "FindRecordDeclsVisitor.h"
#include "ParamVarDeclsVisitor.h"
#include "clang/Basic/SourceManager.h"
#include "ReflectableInterface.h"
#include "clang/Lex/Lexer.h"


namespace clmirror
{
	using namespace clang;

	FindRecordDeclsVisitor::FindRecordDeclsVisitor(const std::string& pCurrentSrcFile, std::vector<std::string>& pUnreflectedFunctions)
		: m_currentSrcFile(pCurrentSrcFile)
		, m_unreflectedFunctions(pUnreflectedFunctions)
	{ }


	bool FindRecordDeclsVisitor::isInUserCode(const NamedDecl* pDecl)
	{
		if (!pDecl) {
			return false;
		}
		
		const SourceManager& SM = pDecl->getASTContext().getSourceManager();
		SourceLocation loc = pDecl->getLocation();
		if (!loc.isValid()) {
			return false;
		}
		
		loc = SM.getSpellingLoc(loc);
		if (SM.isInSystemHeader(loc)) {
			return false;
		}

		return true;
	}



	bool FindRecordDeclsVisitor::isMemberFunctionOrInNamespace(clang::FunctionDecl* pFnDecl)
	{
		if (llvm::isa<clang::CXXRecordDecl>(pFnDecl->getParent())) {
			return true;
		}
		auto currentDecl = pFnDecl->getParent();
		while (currentDecl) {
			if (const clang::NamespaceDecl* namespaceDecl = llvm::dyn_cast<clang::NamespaceDecl>(currentDecl)) {
				return true;
			}
			currentDecl = currentDecl->getParent();
		}
		return false;
	}


	void FindRecordDeclsVisitor::replaceSubString(std::string& pSrcStr, const std::string& pSubstr, const std::string& pReplacestr)
	{
		if (pReplacestr.find(pSubstr) != std::string::npos) {
			return;
		}

		size_t pos = std::string::npos;
		do {
			pos = pSrcStr.find(pSubstr);
			if (pos != std::string::npos) {
				pSrcStr.replace(pos, pSubstr.length(), pReplacestr);
			}
		} while (pos != std::string::npos);
	}


	bool FindRecordDeclsVisitor::isDeclFrmCurrentSource(clang::Decl* pDecl)
	{
		std::string currentSrcFile = m_currentSrcFile;
		std::transform(currentSrcFile.begin(), currentSrcFile.end(), currentSrcFile.begin(),
		[](unsigned char c)->char {
			return (c == '\\') ? '/' : std::tolower(c);
		});

		const auto& srcManager = pDecl->getASTContext().getSourceManager();
		auto fileLoc = srcManager.getFileLoc(pDecl->getBeginLoc());
		auto declSrcFile = srcManager.getFilename(fileLoc).str();
		std::transform(declSrcFile.begin(), declSrcFile.end(), declSrcFile.begin(),
		[](unsigned char c)->char {
			return (c == '\\') ? '/' : std::tolower(c);
		});
		return (currentSrcFile == declSrcFile);
	}

	
	void FindRecordDeclsVisitor::removeSubStrings(std::string& pSrcStr, const std::vector<std::string>& pKeyStrs)
	{
		for (const std::string& keyStr : pKeyStrs)
		{
			if (!keyStr.empty()) 
			{
				size_t pos = pSrcStr.find(keyStr);
				while (pos != std::string::npos)
				{
					size_t charCount = keyStr.size();
					const size_t& endPos = (pos + charCount);
					if (endPos < pSrcStr.size() && pSrcStr.at(endPos) == ' ') {
						charCount++;
					}
					pSrcStr.erase(pos, charCount);
					pos = pSrcStr.find(keyStr, pos);
				}
			}
		}
	}



	std::string FindRecordDeclsVisitor::extractParentTypeName(clang::FunctionDecl* pFnDecl)
	{
		const auto* method = llvm::dyn_cast<clang::CXXMethodDecl>(pFnDecl);
		if (!method)
			return {};

		const clang::CXXRecordDecl* record = method->getParent();

		clang::QualType qt = record->getTypeForDecl()->getCanonicalTypeInternal();

		clang::PrintingPolicy policy(pFnDecl->getASTContext().getLangOpts());
		policy.SuppressScope = false;
		policy.SuppressTagKeyword = true;
		policy.FullyQualifiedName = true;

		std::string result;
		llvm::raw_string_ostream os(result);
		qt.print(os, policy);

		return os.str();
	}




	const std::string FindRecordDeclsVisitor::extractParameterType(clang::ParmVarDecl* pParmVarDecl)
	{
		std::unordered_map<std::string, std::string> templateArgsTypeDefs;
		auto typedefStrValue = getTypeDefAliasForType(pParmVarDecl->getOriginalType(), templateArgsTypeDefs);
		if (typedefStrValue.has_value()) 
		{
			std::string typedefOrgTypeKey;
			const auto& qt = pParmVarDecl->getOriginalType().getCanonicalType().getNonReferenceType();
			if (qt->isFunctionPointerType()) {
				typedefOrgTypeKey = qt.getAsString();
				if (qt.getQualifiers().hasConst()) {
					typedefStrValue.emplace("const " + (typedefStrValue.value()));
				}
			}
			else if (qt->isPointerType()) {
				typedefOrgTypeKey = qt->getPointeeType().getUnqualifiedType().getAsString();
				removeSubStrings(typedefOrgTypeKey, { CONST, ENUM, CLASS, STRUCT });
			}
			else {
				typedefOrgTypeKey = qt.getUnqualifiedType().getAsString();
				removeSubStrings(typedefOrgTypeKey, { CONST, ENUM, CLASS, STRUCT });
			}
			templateArgsTypeDefs.insert(make_pair(typedefOrgTypeKey, typedefStrValue.value()));
		}
		auto typeStr = pParmVarDecl->getOriginalType().getCanonicalType().getAsString();
		removeSubStrings(typeStr, { ENUM, CLASS, STRUCT });
		for (auto itr : templateArgsTypeDefs)
		{
			const auto& tmpTypeStr = itr.first;
			const auto& tmpTypeDefStr = itr.second;
			replaceSubString(typeStr, tmpTypeStr, tmpTypeDefStr);
		}
		replaceSubString(typeStr, "_Bool", "bool");
		return typeStr;
	}


	const std::optional<std::string> FindRecordDeclsVisitor::getTypeDefAliasForType(const QualType& pQType, std::unordered_map<std::string, std::string>& pTemplateTypeDefs)
	{
		const Type* type = pQType.getTypePtrOrNull();
		if (!type) {
			return std::nullopt;
		}

		switch (pQType->getTypeClass())
		{
			case Type::TypeClass::Typedef: {
				return pQType.getAsString();
			}
			case Type::TypeClass::Elaborated: {
				const ElaboratedType* nxtType = dyn_cast<ElaboratedType>(type);
				return getTypeDefAliasForType(nxtType->getNamedType(), pTemplateTypeDefs);
			}
			case Type::TypeClass::LValueReference: {
				const LValueReferenceType* nxtType = dyn_cast<LValueReferenceType>(type);
				return getTypeDefAliasForType(nxtType->getPointeeType(), pTemplateTypeDefs);
			}
			case Type::TypeClass::Pointer: {
				const PointerType* nxtType = dyn_cast<PointerType>(type);
				return getTypeDefAliasForType(nxtType->getPointeeType(), pTemplateTypeDefs);
			}
			case Type::TypeClass::TemplateSpecialization: {
				const TemplateSpecializationType* templateSpclType = dyn_cast<TemplateSpecializationType>(type);
				for (const auto& templateArg : templateSpclType->template_arguments())
				{
					if (templateArg.getKind() == TemplateArgument::ArgKind::Type) 
					{
						std::unordered_map<std::string, std::string> tempTypeDefs;
						auto typeDefStr = getTypeDefAliasForType(templateArg.getAsType(), tempTypeDefs);
						if (typeDefStr.has_value())
						{
							const auto& qt = templateArg.getAsType().getUnqualifiedType().getNonReferenceType().getCanonicalType();
							std::string typeStr = (qt->isPointerType() ? qt->getPointeeType().getUnqualifiedType() : qt).getAsString();
							removeSubStrings(typeStr, { ENUM, CLASS, STRUCT });
							for (auto itr : pTemplateTypeDefs){
								const auto& tmpTypeStr = itr.first;
								const auto& tmpTypeDefStr = itr.second;
								replaceSubString(typeStr, tmpTypeStr, tmpTypeDefStr);
							}
							pTemplateTypeDefs.insert(std::make_pair(typeStr, typeDefStr.value()));
						}
					}
				}
				return std::nullopt;
			}
			default: return std::nullopt;
		}
	}


	bool FindRecordDeclsVisitor::VisitFunctionDecl(FunctionDecl* pFnDecl)
	{
		if (!isInUserCode(pFnDecl) ||
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

		if (!isDeclFrmCurrentSource(pFnDecl)) {
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

		const auto& functionName = pFnDecl->getDeclName().getAsString();
		if (!declSrcFile.empty()) 
		{
			std::vector<std::string> parmTypes;
			const auto& params = pFnDecl->parameters();
			for (unsigned index = 0; index < params.size(); index++)
			{
				if (params[index]->isInAnonymousNamespace()) {
					return true;
				}
				if (params[index]->isInvalidDecl()) {
					m_unreflectedFunctions.push_back(functionName);
					return true;
				}
				parmTypes.push_back(extractParameterType(params[index]));
			}

			MetaKind metaKind = MetaKind::MemberFnNonConst;
			if (llvm::isa<clang::CXXConstructorDecl>(pFnDecl) ||
				llvm::isa<clang::CXXDestructorDecl>(pFnDecl)) {
				metaKind = MetaKind::CtorDtor;
			}
			else if (const auto* method = llvm::dyn_cast<clang::CXXMethodDecl>(pFnDecl)) {
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
			}
			else {
				metaKind = MetaKind::NonMemberFn;
			}
			const std::string recordStr = extractParentTypeName(pFnDecl);
			ReflectableInterface::Instance().addFunctionSignature(metaKind, m_currentSrcFile, declSrcFile,
																  recordStr, functionName, parmTypes);
		}
		return true;
	}
}