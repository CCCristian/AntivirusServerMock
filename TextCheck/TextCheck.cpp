// TextCheck.cpp
//

#include "TextCheck.h"

bool containsString(const std::string& source, const std::string& pattern)
{
	return source.find(pattern) != source.npos;
}

bool textCheck(const std::string& source, const std::vector<std::string>& dictionary)
{
	for (const std::string& str : dictionary)
		if (containsString(source, str))
			return true;
	return false;
}
