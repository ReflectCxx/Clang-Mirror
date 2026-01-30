
#include <map>
#include <vector>
#include <string>

#include "Constants.h"
#include "StringUtils.h"
#include "MetaFunction.h"

namespace clmirror 
{
	std::string MetaFunction::toIdentifierSyntax() const
	{
		std::vector<std::string> typenames = splitQualifiedName(fnRecord);

		std::string syntaxStr;
		for (const auto& typeStr : typenames) {
			syntaxStr.append("\n\nnamespace " + typeStr + " {");
		}

		syntaxStr.append("\n    inline constexpr std::string_view ")
			     .append(fnName)
			     .append(" = \"")
			     .append(fnName)
			     .append("\";");

		for (const auto& typeStr : typenames) {
			syntaxStr.append("\n}");
		}

		return syntaxStr;
	}
}