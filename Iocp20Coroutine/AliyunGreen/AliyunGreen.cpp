#include "pch.h"
#include <WinSock2.h>
#include <windows.h>
#include <winhttp.h>
#include <wchar.h>
#include <wincrypt.h>
#include <comdef.h>
#include <string>
#include <iostream>
#include <format>
#include <unordered_map>
#include <wininet.h>
#include "../jsoncpp-master/include/json/json.h"
#include "../读配置文件/Try读Ini本地机器专用.h"
#include "../读配置文件/文件存取对象.h"



#pragma comment(lib, "Winhttp.lib")
#pragma comment(lib, "Crypt32.lib")

#pragma comment(lib, "../glog/lib/glog.lib")
#ifdef _DEBUG
#pragma comment(lib, "../jsoncpp-master/lib/Debug/jsoncpp.lib")
#else
#endif
using namespace std;
wstring string2wstring(const string& str)
{
	_bstr_t tmp = str.c_str();
	wchar_t* pwchar = (wchar_t*)tmp;
	wstring ret = pwchar;
	return ret;
}

std::string winhttp_client_post(const std::wstring& strHost, const std::wstring& strVerb, const bool bPost, const std::string& strData, const uint32_t u超时秒, const INTERNET_PORT port) {

	DWORD dwBytesWritten = 0;
	BOOL  bResults = FALSE;
	HINTERNET hSession = NULL,
		hConnect = NULL,
		hRequest = NULL;



	// Use WinHttpOpen to obtain a session handle.
	hSession = WinHttpOpen(L"A WinHTTP Example Program/1.0",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS, 0);

	// Specify an HTTP server.
	if (hSession)
		hConnect = WinHttpConnect(hSession, strHost.c_str(),//L"www.wingtiptoys.com",
			port, 0);

	// Create an HTTP Request handle.
	if (hConnect)
		hRequest = WinHttpOpenRequest(hConnect, bPost ? L"POST" : L"GET",
			strVerb.c_str(),
			NULL, WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			WINHTTP_FLAG_SECURE);

	// Set HTTP Options
	DWORD dwTimeOut = u超时秒;
	DWORD dwFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA |
		SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE |
		SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
		SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;

	BOOL bRet = WinHttpSetOption(hRequest, WINHTTP_OPTION_CONNECT_TIMEOUT, &dwTimeOut, sizeof(DWORD));
	LOG_IF(WARNING, !bRet) << "WINHTTP_OPTION_CONNECT_TIMEOUT Error: " << GetLastError();
	bRet = WinHttpSetOption(hSession, WINHTTP_OPTION_RECEIVE_TIMEOUT, &dwTimeOut, sizeof(dwTimeOut));
	LOG_IF(WARNING, !bRet) << "WINHTTP_OPTION_RECEIVE_TIMEOUT Error: " << GetLastError();
	bRet = WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));
	LOG_IF(WARNING, !bRet) << "WINHTTP_OPTION_SECURITY_FLAGS Error: " << GetLastError();
	bRet = WinHttpSetOption(hRequest, WINHTTP_OPTION_CLIENT_CERT_CONTEXT, WINHTTP_NO_CLIENT_CERT_CONTEXT, 0);
	LOG_IF(WARNING, !bRet) << "WINHTTP_OPTION_CLIENT_CERT_CONTEXT Error: " << GetLastError();

	//加上OAuth认证需要的header信息：
	//std::string client_id = "test client id";
	//std::string client_secure = "test client security";
	// client id and secure need base64 encode
	//std::wstring strHeader = L"Content-type:application/x-www-form-urlencoded\r\n";
	std::wstring strHeader = L"Content-Type: application/json";
	//strHeader += L"Authorization: Basic ";
	//strHeader += string2wstring(tmsstring) +L"\r\n"; //tmsstring is client and secure after base64 encoding

	bRet = WinHttpAddRequestHeaders(hRequest, strHeader.c_str(), (DWORD)strHeader.length(), WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE);
	//Send a Request.

	std::string strTmp = "grant_type=client_credentials"; //OAuth认证模式是客户端模式
	if (hRequest)
		bResults = WinHttpSendRequest(hRequest,
			WINHTTP_NO_ADDITIONAL_HEADERS,
			0, WINHTTP_NO_REQUEST_DATA, 0,
			(DWORD)strData.size(), 0);

	// Write data to the server. don't need this step
	if (bResults && !strData.empty())
		bResults = WinHttpWriteData(hRequest, strData.c_str(),
			(DWORD)strData.size(),
			&dwBytesWritten);

	// End the request.
	if (bResults)
		bResults = WinHttpReceiveResponse(hRequest, NULL);

	// Report any errors.
	if (!bResults) {
		auto err = GetLastError();
		switch (err)
		{
		case ERROR_WINHTTP_INCORRECT_HANDLE_STATE: printf("无法执行请求的操作，因为提供的句柄未处于正确的状态。"); break;
		case ERROR_WINHTTP_INCORRECT_HANDLE_TYPE: printf("为此操作提供的句柄类型不正确。"); break;
		case ERROR_WINHTTP_INTERNAL_ERROR: printf("发生了内部错误。"); break;
		case ERROR_NOT_ENOUGH_MEMORY: printf("内存不足，无法完成请求的操作。"); break;
		case ERROR_WINHTTP_INVALID_SERVER_RESPONSE: printf("无法分析服务器响应。"); break;
		case ERROR_INVALID_HANDLE:printf("The handle is invalid."); break;
		case ERROR_INVALID_PARAMETER:printf("The parameter is incorrect."); break;
		case ERROR_INTERNET_TIMEOUT:printf("The parameter is incorrect."); break;
		default:
			printf("Error %d has occurred.\n", err);
			break;
		}
		return "";
	}

	//接收服务器返回数据
	//if (bRet)
	std::string strRet;
	{
		char* pszOutBuf;
		DWORD dwSize = 0;
		DWORD dwDownLoaded = 0;
		std::string strJson; //返回的是Json格式

		do
		{
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
			{
				//error log
			}
			pszOutBuf = new char[dwSize + 1];
			ZeroMemory(pszOutBuf, dwSize + 1);

			if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuf, dwSize, &dwDownLoaded))
			{
				//error log
			}

			strJson += pszOutBuf;
		} while (dwSize > 0);
		OutputDebugStringA(strJson.c_str());
		strRet = strJson;
	}


	// Close any open handles.
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);

	return strRet;
}

#include "../IocpNetwork/StrConv.h"
#include "AliyunGreen.h"
std::wstring StrToW(const std::string& str)
{
	return std::wstring(str.begin(), str.end());
}
class 耗时 final
{
public:
	耗时(const std::string& str) :m_str(str)
	{
		m_t = std::chrono::high_resolution_clock::now();

	}
	~耗时()
	{
		auto t = std::chrono::high_resolution_clock::now();
		auto d = t - m_t;
		using namespace std;
		if (d > 500ms)
			LOG(WARNING) << "耗时：" << std::chrono::duration_cast<std::chrono::milliseconds>(d).count() << "毫秒";
	}
	std::chrono::high_resolution_clock::time_point m_t;
	const std::string m_str;
};

/// <summary>
/// 阿里内容安全
/// </summary>
struct 阿里绿化结果缓存
{
	std::unordered_map<std::string, std::string> m_map;
	MSGPACK_DEFINE(m_map);
};


bool AliyunGreen::Check(const std::string& refContentGbk)
{
	耗时 _(refContentGbk + "AliyunGreen::Check");
	const char sz缓存文件[] = "阿里绿化结果缓存";

	static 阿里绿化结果缓存 s_阿里绿化结果缓存;
	if (s_阿里绿化结果缓存.m_map.empty())
	{
		s_阿里绿化结果缓存 = 从文件里读出对象<阿里绿化结果缓存>(sz缓存文件);
		LOG(INFO) << "从文件里读出对象<阿里绿化结果缓存>成功,size=" << s_阿里绿化结果缓存.m_map.size();
		for (const auto& [k, v] : s_阿里绿化结果缓存.m_map)
		{
			LOG(INFO) << k << ":" << v;
		}
	}

	std::string strGbk响应;
	auto iterFind = s_阿里绿化结果缓存.m_map.find(refContentGbk);
	if (iterFind == s_阿里绿化结果缓存.m_map.end())
	{
		std::string strHttpsHost;
		std::string strHttpsVerb;
		uint32_t u超时秒(0);
		uint32_t uPort(0);
		Try读Ini本地机器专用(strHttpsHost, "Aliyun", "HttpsHost");
		Try读Ini本地机器专用(strHttpsVerb, "Aliyun", "HttpsVerb");
		Try读Ini本地机器专用(u超时秒, "Aliyun", "超时秒");
		Try读Ini本地机器专用(uPort, "Aliyun", "Port");
		if (0 == u超时秒)
		{
			LOG(WARNING) << "不判断文本违规，直接允许";
			return true;
		}
		//const auto strToken = winhttp_client_post(StrToW(strHttpsHost), StrToW(strHttpsVerb), true, std::format("content={0}", StrConv::GbkToUtf8(refContentGbk)), u超时秒 * CLOCKS_PER_SEC, uPort);// / wxa / msg_sec_check");
		const auto strToken = winhttp_client_post(StrToW(strHttpsHost), StrToW(strHttpsVerb), true, std::format("{{\"content\"\:\"{0}\"}}", StrConv::GbkToUtf8(refContentGbk)), u超时秒 * CLOCKS_PER_SEC, uPort);// / wxa / msg_sec_check");
		strGbk响应 = StrConv::Utf8ToGbk(strToken);
		if (strToken.size() > 10 || strToken.empty())//<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">< html ><head> < title>504 Gateway Time - out< / title>< / head><body>< h1>504 Gateway Time - out< / h1><p>The gateway did not receive a timely response from the upstream server or application.<hr / >Powered by Tengine< / body>< / html>
		{
			LOG(INFO) << "检查报错，当成合规处理:" << strToken;
			return true;
		}

		s_阿里绿化结果缓存.m_map[refContentGbk] = strGbk响应;
		写对象进文件(s_阿里绿化结果缓存, sz缓存文件);
		LOG(INFO) << "返回新的:" << strGbk响应;
	}
	else
	{
		strGbk响应 = iterFind->second;
		LOG(INFO) << "返回缓存的:" << strGbk响应;
	}


	if (strGbk响应 != "\"none\"")
		return false;

	return true;
}