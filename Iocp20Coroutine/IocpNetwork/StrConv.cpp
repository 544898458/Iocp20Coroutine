
#include "StrConv.h"
#include <Windows.h>

namespace StrConv
{
	std::string GbkToUtf8(const char* src_str)
	{
		int len = MultiByteToWideChar(CP_ACP, 0, src_str, -1, NULL, 0);
		wchar_t* wstr = new wchar_t[len + 1];
		memset(wstr, 0, len + 1);
		MultiByteToWideChar(CP_ACP, 0, src_str, -1, wstr, len);
		len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
		char* str = new char[len + 1];
		memset(str, 0, len + 1);
		WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
		auto strTemp = str;
		if (wstr) delete[] wstr;
		if (str) delete[] str;
		return strTemp;
	}

	std::string Utf8ToGbk(const std::string& str)
	{
#if defined(_WIN32) || defined(_MSC_VER) || defined(WIN64) 
		// calculate length
		int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
		wchar_t* wsGbk = new wchar_t[len + 1ull];
		// set to '\0'
		memset(wsGbk, 0, len + 1ull);
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wsGbk, len);
		len = WideCharToMultiByte(CP_ACP, 0, wsGbk, -1, NULL, 0, NULL, NULL);
		char* csGbk = new char[len + 1ull];
		memset(csGbk, 0, len + 1ull);
		WideCharToMultiByte(CP_ACP, 0, wsGbk, -1, csGbk, len, NULL, NULL);
		std::string res(csGbk);

		if (wsGbk)
		{
			delete[] wsGbk;
		}

		if (csGbk)
		{
			delete[] csGbk;
		}

		return res;
#elif defined(__linux__) || defined(__GNUC__)
		size_t len = str.size() * 2 + 1;
		char* temp = new char[len];
		if (EncodingConvert("utf-8", "gb2312", const_cast<char*>(str.c_str()),
			str.size(), temp, len) >= 0)
		{
			std::string res;
			res.append(temp);
			delete[] temp;
			return res;
		}
		else
		{
			delete[] temp;
			return str;
		}

#else
		std::cerr << "Unhandled operating system." << std::endl;
		return str;
#endif
	}

}