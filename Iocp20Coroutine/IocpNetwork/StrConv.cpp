#include "pch.h"

#include "StrConv.h"
#include <Windows.h>
#include <assert.h>

namespace StrConv
{
	std::string GbkToUtf8(const std::string& src_str)
	{
		const auto needWideCount = MultiByteToWideChar(CP_ACP, 0, src_str.c_str(), -1, NULL, 0);//����ɹ����򷵻�д�� �� lpWideCharStr ָʾ�Ļ��������ַ����� ��������ɹ��� cchWideChar Ϊ 0���򷵻�ֵ�� lpWideCharStr ָʾ�Ļ���������Ĵ�С�����ַ�Ϊ��λ����
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
			str.resize(str.size() - 1);//utf8����Ҫ0��β��
		}

		return str;
	}

	std::string Utf8ToGbk(const std::string& strUtf8)
	{
#if defined(_WIN32) || defined(_MSC_VER) || defined(WIN64) 
		//1��UTF8 ��ҪתΪunicode  windows utf16
		//1.1 ͳ��ת������ֽ���
		int len = MultiByteToWideChar(CP_UTF8,	//ת���ĸ�ʽ
			0,							//Ĭ�ϵ�ת����ʽ
			strUtf8.c_str(),						//������ֽ�
			-1,							//������ַ�����С -1��\0����  �Լ�ȥ��
			0,							//������������ͳ��ת������ֽ�����
			0							//����Ŀռ��С
		);
		if (len <= 0)
		{
			return std::string();
		}
		std::wstring wstr; //��wstring�洢��
		wstr.resize(len);//�����С
		//��ʼд��ȥ 
		MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, (wchar_t*)wstr.data(), len);


		//2�� unicode ת GBK 
		len = WideCharToMultiByte(CP_ACP, 0, (wchar_t*)wstr.data(), -1, 0, 0,
			0, //ʧ�����Ĭ���ַ�
			0 //�Ƿ�ʹ��Ĭ�����  0 false
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