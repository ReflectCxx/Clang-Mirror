#pragma once

#include <vector>
#include <string>
#include <unordered_map>

namespace clmirror {
	struct UserType;
	struct ReflectionMeta;
}

namespace clmirror 
{
	class ReflectableInterface
	{
		std::unordered_map<std::string, UserType> m_metaTypes;
		std::unordered_multimap<std::string, ReflectionMeta> m_metaFns;

		ReflectableInterface();
		~ReflectableInterface();

		void printRecordTypeIds(std::fstream& pOut);

		void printFreeFunctionIds(std::fstream& pOut);

		void addReflectionMetaAsRecord(const ReflectionMeta& pReflMeta);

	public:

		ReflectableInterface(const ReflectableInterface&) = delete;
		ReflectableInterface& operator=(const ReflectableInterface&) = delete;

		static ReflectableInterface& Instance();

		void addFunctionSignature(MetaKind pMetaKind, const std::string& pSrcFile,
								  const std::string& pHeaderFile, const std::string& pRecord,
								  const std::string& pFunctionName, const std::vector<std::string>& pParmTypes);
		void dump();
	};
}