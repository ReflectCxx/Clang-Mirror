#pragma once

#include <vector>
#include <string>
#include <unordered_map>

namespace clmirror {
	struct RtlRecord;
	struct RtlFunction;
	class RtlCodeGenerator;
}

namespace clmirror 
{
	class RtlCodeManager
	{
		std::vector<RtlCodeGenerator> m_codeGens;
		std::unordered_map<std::string, RtlRecord> m_metaTypes;
		std::unordered_multimap<std::string, RtlFunction> m_metaFns;

		RtlCodeManager();
		~RtlCodeManager();

		void printRecordTypeIds(std::fstream& pOut);

		void printFreeFunctionIds(std::fstream& pOut);

		void printRegistrationDecls(std::fstream& pOut);

		void addReflectionMetaAsRecord(const RtlFunction& pReflMeta);

		void dumpMetadataIds(std::fstream& pOut);

		void dumpRegistrationDecls(std::fstream& pOut);

	public:

		RtlCodeManager(RtlCodeManager&&) = delete;
		RtlCodeManager(const RtlCodeManager&) = delete;
		RtlCodeManager& operator=(RtlCodeManager&&) = delete;
		RtlCodeManager& operator=(const RtlCodeManager&) = delete;
		
		RtlCodeGenerator& initCodeGenerator(const std::string& pSrcFile);

		static RtlCodeManager& Instance();

		void addFunctionSignature(MetaKind pMetaKind, const std::string& pHeaderFile, const std::string& pRecord,
								  const std::string& pFunctionName, const std::vector<std::string>& pParmTypes);

		void generateRegistrationCode();
	};
}