#include "pch.h"
#include <windows.h>
#include <winhttp.h>
#include <wchar.h>
#include <wincrypt.h>
#include <comdef.h>
#include <string>
#include <iostream>
#include <format>
#include "../jsoncpp-master/include/json/json.h"
#include "../读配置文件/Try读Ini本地机器专用.h"
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

std::string winhttp_client_post(const std::wstring& strHost, const std::wstring& strVerb, const bool bPost, const std::string& strData) {

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
			INTERNET_DEFAULT_HTTPS_PORT, 0);

	// Create an HTTP Request handle.
	if (hConnect)
		hRequest = WinHttpOpenRequest(hConnect, bPost ? L"POST" : L"GET",
			strVerb.c_str(),
			NULL, WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			WINHTTP_FLAG_SECURE);

	// Set HTTP Options
	DWORD dwTimeOut = 3000;
	//DWORD dwFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA |
	//	SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE |
	//	SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
	//	SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;

	BOOL bRet = WinHttpSetOption(hRequest, WINHTTP_OPTION_CONNECT_TIMEOUT, &dwTimeOut, sizeof(DWORD));
	//bRet = WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));
	//bRet = WinHttpSetOption(hRequest, WINHTTP_OPTION_CLIENT_CERT_CONTEXT, WINHTTP_NO_CLIENT_CERT_CONTEXT, 0);

	//加上OAuth认证需要的header信息：
	//std::string client_id = "test client id";
	//std::string client_secure = "test client security";
	// client id and secure need base64 encode
	std::wstring strHeader = L"Content-type:application/x-www-form-urlencoded\r\n";
	//strHeader += L"Authorization: Basic ";
	//strHeader += string2wstring(tmsstring) +L"\r\n"; //tmsstring is client and secure after base64 encoding

	bRet = WinHttpAddRequestHeaders(hRequest, strHeader.c_str(), strHeader.length(), WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE);
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
bool AliyunGreen::Check(const std::string& refContentGbk)
{

	const auto strToken = winhttp_client_post(L"https.iotlabor.cn", L"public/index.php/index/admin/green", true, std::format("content={0}", StrConv::GbkToUtf8(refContentGbk)));// / wxa / msg_sec_check");
	const auto strGbk = StrConv::Utf8ToGbk(strToken);
	//std::cout << strGbk;
	return strGbk == "none";
}