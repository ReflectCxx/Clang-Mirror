
#include <map>
#include <vector>
#include <string>

#include "Constants.h"
#include "StringUtils.h"
#include "ReflectionMeta.h"

namespace clmirror 
{
	std::string ReflectionMeta::toFunctionIdentifierSyntax() const
	{
		std::vector<std::string> typenames = splitQualifiedName(m_function);
		std::string fnName = typenames.back();
		typenames.pop_back();

		std::string syntaxStr = "\nnamespace function {";
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


	std::string ReflectionMeta::toMethodIdentifierSyntax() const
	{
		std::vector<std::string> typenames = splitQualifiedName(m_record);

		std::string syntaxStr = "\nnamespace type {";
		for (const auto& typeStr : typenames) {
			syntaxStr.append("\nnamespace " + typeStr + " {");
		}

		syntaxStr.append("\nnamespace method {")
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


	std::string ReflectionMeta::toRecordIdentifierSyntax() const
	{
		std::vector<std::string> typenames = splitQualifiedName(m_record);

		std::string syntaxStr = "\nnamespace type {";
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
}