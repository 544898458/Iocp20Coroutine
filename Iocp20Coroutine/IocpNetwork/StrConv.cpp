#include "pch.h"

#include "StrConv.h"
#include <Windows.h>
#include <assert.h>

namespace StrConv
{
	std::string GbkToUtf8(const std::string& src_str)
	{
		const auto needWideCount = MultiByteToWideChar(CP_ACP, 0, src_str.c_str(), -1, NULL, 0);//如果成功，则返回写入 到 lpWideCharStr 指示的缓冲区的字符数。 如果函数成功且 cchWideChar 为 0，则返回值是 lpWideCharStr 指示的缓冲区所需的大小（以字符为单位）。
		std::wstring wStr;
		wStr.resize(needWideCount);
		const auto writtenWideCount = MultiByteToWideChar(CP_ACP, 0, src_str.c_str(), -1, (LPWSTR)wStr.c_str(), needWideCount);
		assert(writtenWideCount == needWideCount);

		const auto needCharCount = WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)wStr.c_str(), -1, NULL, 0, NULL, NULL);
		std::string str;
		str.resize(needCharCount);
		const auto writtenCharCount = WideCharToMultiByte(CP_UTF8, 0, wStr.c_str(), wStr.size(), (LPSTR)str.c_str(), needCharCount, NULL, NULL);
		assert(writtenCharCount == needCharCount);
		
		if (!str.empty() && str.back() == 0)
		{
			str.resize(str.size() - 1);//utf8不需要0结尾符
		}

		return str;
	}

	std::string Utf8ToGbk(const std::string& strUtf8)
	{
#if defined(_WIN32) || defined(_MSC_VER) || defined(WIN64) 
		//1、UTF8 先要转为unicode  windows utf16
		//1.1 统计转换后的字节数
		int len = MultiByteToWideChar(CP_UTF8,	//转换的格式
			0,							//默认的转换方式
			strUtf8.c_str(),						//输入的字节
			-1,							//输入的字符串大小 -1找\0结束  自己去算
			0,							//输出（不输出，统计转换后的字节数）
			0							//输出的空间大小
		);
		if (len <= 0)
		{
			return std::string();
		}
		std::wstring wstr; //用wstring存储的
		wstr.resize(len);//分配大小
		//开始写进去 
		MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, (wchar_t*)wstr.data(), len);


		//2、 unicode 转 GBK 
		len = WideCharToMultiByte(CP_ACP, 0, (wchar_t*)wstr.data(), -1, 0, 0,
			0, //失败替代默认字符
			0 //是否使用默认替代  0 false
		);
		if (len <= 0)
		{
			return std::string();
		}
		std::string utf8;
		utf8.resize(len);
		WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, (char*)utf8.data(), len, 0, 0);

		return utf8;
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