#pragma once
#include <string>
namespace StrConv
{
	std::string GbkToUtf8(const char* src_str);
	std::string Utf8ToGbk(const std::string& str);
};