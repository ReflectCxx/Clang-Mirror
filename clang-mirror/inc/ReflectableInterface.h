#pragma once

#include <map>
#include <vector>
#include <string>

#include "MetaFunction.h"

namespace clmirror 
{
	class ReflectableInterface
	{
		using FuncSignature = std::multimap<std::string, std::vector<std::string> >;
		using FuncHeaderMap = std::map<std::string, FuncSignature>;
		std::map<std::string, FuncHeaderMap> m_functionSignatureMap;

		std::vector<MetaFunction> m_metaFns;

		ReflectableInterface();
		~ReflectableInterface();

	public:

		ReflectableInterface(const ReflectableInterface&) = delete;
		ReflectableInterface& operator=(const ReflectableInterface&) = delete;

		static ReflectableInterface& Instance();

		void addFunctionSignature(MetaKind pMetaKind, const std::string& pSrcFile, const std::string& pHeaderFile,
								  const std::string& pRecord, const std::string& pFunctionName,
								  const std::string& pReturn, const std::vector<std::string>& pParmTypes);
		void dump();
	};
}