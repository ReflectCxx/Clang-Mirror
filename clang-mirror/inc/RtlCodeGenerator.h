#pragma once

#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>

#include "ReflectionMeta.h"

namespace clmirror 
{
	class RtlCodeManager;
}

namespace clmirror
{
	class RtlCodeGenerator 
	{
		const std::string m_srcFile;
		std::unordered_set<std::string> m_incFiles;
		std::unordered_map<std::string, RtlRecord> m_metaTypes;
		std::unordered_multimap<std::string, RtlFunction> m_metaFns;

		RtlCodeGenerator(const std::string& pSrcFile);
		

	public:

		GETTER_CREF(std::string, SrcFile, m_srcFile)

		RtlCodeGenerator() = delete;
		RtlCodeGenerator(RtlCodeGenerator&&) = default;
		RtlCodeGenerator(const RtlCodeGenerator&) = default;
		RtlCodeGenerator& operator=(RtlCodeGenerator&&) = delete;
		RtlCodeGenerator& operator=(const RtlCodeGenerator&) = delete;

		friend RtlCodeManager;
	};
}