// BinaryCheck.cpp
//

#include "BinaryCheck.h"

bool binaryCheck(const std::string& source, const std::vector<std::pair<const char*, int>>& dictionary)
{
	/*
	for (const std::pair<const char*, int>& pair : dictionary)
	{
		BitFields fieldList(pair.first, pair.second);
		if (fieldList.isContainedIn(source))
			return true;
	}
	*/
	return false;
}

Source::Source(const std::string& source): source(source.c_str()), byteCount(source.length()) {}
Source::Source(const char *source, int byteCount): source(source), byteCount(byteCount) {}
int BitFields::countBytes(int bitCount)
{
	if (bitCount == 0) return 0;
	return (bitCount - 1) / 8;
}
BitFields::BitFields(const char *base, int bitCount): bitCount(bitCount)
{
	int byteCount = countBytes(bitCount);
	for (int i = 0; i < 8; i++)
	{
		sizes[i] = countBytes(bitCount + i) > byteCount ? byteCount + 1 : byteCount;
		fields[i] = new char[sizes[i]];
		fields[i][0] = base[0] >> i;
		if (sizes[i] == 1) continue;
		fields[i][sizes[i] - 1] = base[sizes[i] - 2] << (8 - i);
		for (int j = 1; j < sizes[i] - 1; j++)
		{
			fields[i][j] = (fields[i][j] >> i) | (fields[i][j - 1] << (8 - i));
		}
	}
}
BitFields::~BitFields()
{
	for (int i = 0; i < 8; i++)
	{
		delete[] fields[i];
	}
}
bool BitFields::isContainedIn(const Source& source)
{
	for (int field = 0; field < 8; field++)
	{
		for (int offset = 0; offset <= source.byteCount - sizes[field]; offset++)
		{
			bool foundDifference = false;
			// Not applicable if the searched bit field is less than 8 bits wide.
			if ((source.source[offset] & ((~0) >> field)) != fields[field][0])
				foundDifference = true;
			if ((source.source[offset + source.byteCount - 1] & ((~0) << (8 - field))) != fields[field][sizes[field] - 1])
				foundDifference = true;
			for (int j = 1; j < sizes[field] - 1; j++)
				if (source.source[offset + j] != fields[field][j])
					foundDifference = true;
			if (!foundDifference)
				return true;
		}
	}
	return false;
}
