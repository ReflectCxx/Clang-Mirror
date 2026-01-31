#include <iostream>
#include <mutex>
#include <fstream>
#include <filesystem>
#include <unordered_set>

#include "Logger.h"
#include "ReflectionMeta.h"
#include "RtlCodeManager.h"
#include "RtlCodeGenerator.h"

namespace clmirror 
{
    RtlCodeManager::RtlCodeManager() 
    { }

    RtlCodeManager::~RtlCodeManager() 
    { }

    RtlCodeManager& RtlCodeManager::Instance()
    {
        static RtlCodeManager instance;
        return instance;
    }

    RtlCodeGenerator& RtlCodeManager::initCodeGenerator(const std::string& pSrcFile)
    {
        static std::mutex mutex;
        m_codeGens.emplace_back(RtlCodeGenerator(pSrcFile));
        return m_codeGens.back();
    }

    void RtlCodeManager::dumpMetadataIds(std::fstream& pOut)
    {
        pOut << "\n#pragma once"
                "\n#include <string_view>\n"
                "\nnamespace rtcl {\n";
        printFreeFunctionIds(pOut);
        printRecordTypeIds(pOut);
        pOut << "\n}";
    }

    void RtlCodeManager::dumpRegistrationDecls(std::fstream& pOut)
    {
        pOut << "\n#pragma once"
                "\n#include <vector>\n"
                "\nnamespace rtl { class Function; }\n"
                "\nnamespace rtcl {\n";
        printRegistrationDecls(pOut);
        pOut << "\n}";
    }

    void RtlCodeManager::printFreeFunctionIds(std::fstream& pOut)
    {
        std::unordered_set<std::string> seen;
        for (auto it = m_metaFns.begin(); it != m_metaFns.end(); ++it)
        {
            const std::string& key = it->first;
            if (!seen.insert(key).second) {
                continue;
            }
            pOut << it->second.toFunctionIdentifierSyntax() << "\n";
        }
    }


    void RtlCodeManager::printRegistrationDecls(std::fstream& pOut)
    {
        pOut << "\nnamespace " + std::string(NS_REGISTRATION) + " {"
                "\n    " + std::string(DECL_INIT_REGIS) + "\n}\n";

        for (const auto& itr : m_metaTypes) {

            std::unordered_set<std::string> seen;
            const auto& methodMap = itr.second.methods;
            const auto& fnMeta = methodMap.begin()->second;

            pOut << fnMeta.toRegistrationDeclSyntax() << "\n";
        }
    }


    void RtlCodeManager::printRecordTypeIds(std::fstream& pOut)
    {
        for (const auto& itr : m_metaTypes) {

            std::unordered_set<std::string> seen;
            const auto& methodMap = itr.second.methods;
            const auto& fnMeta = methodMap.begin()->second;

            pOut << fnMeta.toRecordIdentifierSyntax() << "\n";
            for (auto it = methodMap.begin(); it != methodMap.end(); ++it)
            {
                const std::string& key = it->first;
                if (!seen.insert(key).second) {
                    continue;
                }
                pOut << it->second.toMethodIdentifierSyntax() << "\n";
            }
            pOut << "\n";
        }
    }


    void RtlCodeManager::addReflectionMetaAsRecord(const RtlFunction& pReflMeta)
    {
        auto& userType = [&]()-> RtlRecord&
        {
            const auto& itr = m_metaTypes.find(pReflMeta.m_record);
            if (itr == m_metaTypes.end()) 
            {
                auto& userType = m_metaTypes.emplace(pReflMeta.m_record,
                    RtlRecord{
                        .typeStr = pReflMeta.m_record,
                        .methods = RtlRecord::MemberFnsMap()
                    }).first->second;
                return userType;
            }
            else {
                auto& userType = itr->second;
                return userType;
            }
        }();
        userType.methods.emplace(pReflMeta.m_function, pReflMeta);
    }


    void RtlCodeManager::addFunctionSignature(MetaKind pMetaKind, const std::string& pHeaderFile, const std::string& pRecord,
                                              const std::string& pFunctionName, const std::vector<std::string>& pParmTypes)
    {
        static std::mutex mutex;
        std::lock_guard<std::mutex> lock(mutex);

        if (pMetaKind == MetaKind::NonMemberFn)
        {
            m_metaFns.emplace(pFunctionName, (RtlFunction{
                    .m_metaKind = pMetaKind,
                    .m_header = pHeaderFile,
                    .m_record = pRecord,
                    .m_function = pFunctionName,
                    .m_argTypes = pParmTypes
            }));
        }
        else if (pMetaKind != MetaKind::None)
        {
            addReflectionMetaAsRecord(RtlFunction{
                    .m_metaKind = pMetaKind,
                    .m_header = pHeaderFile,
                    .m_record = pRecord,
                    .m_function = pFunctionName,
                    .m_argTypes = pParmTypes
            });
        }
    }


    void RtlCodeManager::generateRegistrationCode()
    {
        {
            const std::string fileStr = std::filesystem::current_path().string() + "/" + std::string(META_ID_HEADER);
            std::fstream fout(fileStr, std::ios::out);
            if (!fout.is_open()) {
                Logger::outException("Error opening file for writing!");
                return;
            }

            dumpMetadataIds(fout);
            fout.flush();
            fout.close();

            if (fout.fail() || fout.bad()) {
                Logger::outException("Error closing file:" + std::string(META_ID_HEADER));
                return;
            }
            Logger::out("generated file : " + fileStr);
        } {
            const std::string fileStr = std::filesystem::current_path().string() + "/" + std::string(REGISTRATION_HEADER);
            std::fstream fout(fileStr, std::ios::out);
            if (!fout.is_open()) {
                Logger::outException("Error opening file for writing!");
                return;
            }

            dumpRegistrationDecls(fout);
            fout.flush();
            fout.close();

            if (fout.fail() || fout.bad()) {
                Logger::outException("Error closing file:" + std::string(META_ID_HEADER));
                return;
            }
            Logger::out("generated file : " + fileStr);
        }
        Logger::out("Number of reflectable types generated: " + std::to_string(m_metaTypes.size()));
    }
}
