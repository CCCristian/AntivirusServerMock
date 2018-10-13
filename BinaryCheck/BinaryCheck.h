#pragma once

#include <vector>
#include <string>

#ifdef BINARYCHECK_EXPORTS
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif

struct Source
{
	const char *source;
	int byteCount;
	EXPORT Source(const std::string& source);
	EXPORT Source(const char *source, int byteCount);
	void operator =(const Source&) = delete;
};
struct BitFields
{
	char *fields[8];
	int sizes[8];
	int bitCount;
	BitFields(const char *base, int bitCount);
	BitFields(const BitFields&) = delete;
	void operator =(const BitFields&) = delete;
	~BitFields();
	/** Number of bytes bitCount bits fit into. */
	int countBytes(int bitCount);
	bool isContainedIn(const Source& source);
};

EXPORT bool binaryCheck(const std::string& source, const std::vector<std::pair<const char*, int>>& dictionary);