#include <iostream>
#include <mutex>
#include <fstream>
#include <filesystem>


#include "Logger.h"
#include "ReflectableInterface.h"


namespace {

	std::mutex g_mutex;
}


namespace clmirror 
{
	ReflectableInterface::ReflectableInterface() {

	}


	ReflectableInterface::~ReflectableInterface() {

	}


	ReflectableInterface& ReflectableInterface::Instance()
	{
		static ReflectableInterface instance;
		return instance;
	}


	void ReflectableInterface::addFunctionSignature(MetaKind pMetaKind, const std::string& pSrcFile, const std::string& pHeaderFile,
													const std::string& pRecord, const std::string& pFunctionName,
													const std::string& pReturn, const std::vector<std::string>& pParmTypes)
	{
		std::lock_guard<std::mutex> lock(g_mutex);

		const auto& srcItr = m_functionSignatureMap.find(pSrcFile);
		if (srcItr == m_functionSignatureMap.end()) 
		{
			FuncSignature funcSigMap;
			FuncHeaderMap funcHeaderMap;
			funcSigMap.emplace(pFunctionName, pParmTypes);
			funcHeaderMap.emplace(pHeaderFile, funcSigMap);
			m_functionSignatureMap.emplace(pSrcFile, funcHeaderMap);
		}
		else 
		{
			auto& funcHeaderMap = srcItr->second;
			const auto& headerItr = funcHeaderMap.find(pHeaderFile);
			if (headerItr == funcHeaderMap.end()) {
				FuncSignature signatureMap;
				signatureMap.emplace(pFunctionName, pParmTypes);
				funcHeaderMap.emplace(pHeaderFile, signatureMap);
			}
			else {
				headerItr->second.emplace(pFunctionName, pParmTypes);
			}
		}

		m_metaFns.push_back(
			MetaFunction{
				.fnType = pMetaKind,
				.fnHeader = pHeaderFile,
				.fnSource = pSrcFile,
				.fnName = pFunctionName,
				.fnRecord = pRecord,
				.fnReturn = pReturn,
				.fnArgs = pParmTypes
			});
	}


	void ReflectableInterface::dump()
	{
		const std::string fileStr = std::filesystem::current_path().string() + "/" + CL_REFLECT_INTERFACE;
		std::fstream fout(fileStr, std::ios::out);

		if (!fout.is_open()) {
			Logger::outException("Error opening file for writing!");
			return;
		}
		
		fout << "\n"
			"\n#pragma once"
			"\n#include <string_view>\n\n"
			"\nnamespace rtcl {\n";

		for (const MetaFunction& fn : m_metaFns) {
			if (fn.fnType != MetaKind::CtorDtor) {
				fout << fn.toIdentifierSyntax();
			}
		}
		fout << "\n\n}";

		fout.flush();
		fout.close();
		if (fout.fail() || fout.bad()) {
			Logger::outException("Error closing file:" + std::string(CL_REFLECT_INTERFACE));
			return;
		}

		Logger::out("Number of reflectable functions generated: " + std::to_string(m_metaFns.size()));
		//Logger::out("Number of headerfiles shortlisted, containing reflectable functions declarations: " + std::to_string(m_functionSignatureMap.size()));
		Logger::out("Reflection interface file generated : " + fileStr);
	}
}
