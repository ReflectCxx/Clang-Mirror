
#pragma once

#include<string>
#include<vector>

namespace clmirror {

	static std::vector<std::string> splitQualifiedName(const std::string& qname)
	{
		std::vector<std::string> parts;

		size_t start = 0;
		while (true)
		{
			size_t pos = qname.find("::", start);
			if (pos == std::string::npos)
			{
				parts.emplace_back(qname.substr(start));
				break;
			}

			parts.emplace_back(qname.substr(start, pos - start));
			start = pos + 2;
		}

		return parts;
	}
}