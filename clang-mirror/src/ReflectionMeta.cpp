
#include <map>
#include <vector>
#include <string>

#include "Constants.h"
#include "StringUtils.h"
#include "ReflectionMeta.h"

namespace clmirror 
{
    std::string RtlFunction::toFunctionIdentifierSyntax() const
    {
        std::vector<std::string> typenames = splitQualifiedName(m_function);
        std::string fnName = typenames.back();
        typenames.pop_back();

        std::string syntaxStr = "\nnamespace " + std::string(NS_FUNCTION) + " {";
        for (const auto& typeStr : typenames) {
            syntaxStr.append("\nnamespace " + typeStr + " {");
        }

        syntaxStr.append("\n    inline constexpr std::string_view ")
                 .append(fnName)
                 .append(" = \"")
                 .append(m_function)
                 .append("\";\n");

        for (auto& _ : typenames) {
            syntaxStr.append("}");
        }
        syntaxStr.append("}");
        return syntaxStr;
    }


    std::string RtlFunction::toMethodIdentifierSyntax() const
    {
        std::vector<std::string> typenames = splitQualifiedName(m_record);

        std::string syntaxStr = "\nnamespace " + std::string(NS_TYPE) + " {";
        for (const auto& typeStr : typenames) {
            syntaxStr.append("\nnamespace " + typeStr + " {");
        }

        syntaxStr.append("\nnamespace " + std::string(NS_METHOD) + " {")
                 .append("\n    inline constexpr std::string_view ")
                 .append(m_function)
                 .append(" = \"")
                 .append(m_function)
                 .append("\";\n}");

        for (auto& _ : typenames) {
            syntaxStr.append("}");
        }
        syntaxStr.append("}");
        return syntaxStr;
    }


    std::string RtlFunction::toRecordIdentifierSyntax() const
	{
        std::vector<std::string> typenames = splitQualifiedName(m_record);
        
        std::string syntaxStr = "\nnamespace " + std::string(NS_TYPE) + " {";
        for (const auto& typeStr : typenames) {
            syntaxStr.append("\nnamespace " + typeStr + " {");
        }
        
        syntaxStr.append("\n    inline constexpr std::string_view ")
                 .append("id")
                 .append(" = \"")
                 .append(m_record)
                 .append("\";\n");
        
        for (auto& _ : typenames) {
            syntaxStr.append("}");
        }
        syntaxStr.append("}");
        return syntaxStr;
    }


    std::string RtlFunction::toRegistrationDeclSyntax() const
    {
        std::vector<std::string> typenames = splitQualifiedName(m_record);

        std::string syntaxStr = "\nnamespace " + std::string(NS_REGISTRATION) + " {"
                                "\nnamespace " + std::string(NS_TYPE) + " {";

        for (const auto& typeStr : typenames) {
            syntaxStr.append("\nnamespace " + typeStr + " {");
        }

        syntaxStr.append("\n    " + std::string(DECL_INIT_REGIS) + "\n");

        for (auto& _ : typenames) {
            syntaxStr.append("}");
        }
        syntaxStr.append("}}");
        return syntaxStr;
    }
}