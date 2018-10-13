#pragma once

#include <string>
#include <vector>

#ifdef TEXTCHECK_EXPORTS
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif

EXPORT bool textCheck(const std::string& source, const std::vector<std::string>& dictionary);