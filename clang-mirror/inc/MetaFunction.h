
#include <map>
#include <vector>
#include <string>

#include "Constants.h"

namespace clmirror 
{
	struct MetaFunction 
	{
		MetaKind fnType;
		
		std::string fnHeader;
		std::string fnSource;

		std::string fnName;
		std::string fnRecord;
		std::string fnReturn;
		std::vector<std::string> fnArgs;

		std::string toIdentifierSyntax() const;
	};
}