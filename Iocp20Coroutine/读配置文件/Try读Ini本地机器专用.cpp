#include "pch.h"
#include "Try读Ini本地机器专用.h"
#include <unordered_map>
#include <Windows.h>

bool Try读Ini本地机器专用(std::string& refInOut, const std::string& strAppName, const std::string& strKey)
{
	using namespace std;
	static unordered_map <string, unordered_map<string, string> > s_map;
	auto itFindApp = s_map.find(strAppName);
	if (s_map.end() != itFindApp)
	{
		auto& refMapApp = itFindApp->second;
		auto iterFindKey = refMapApp.find(strKey);
		if (refMapApp.end() != iterFindKey)
		{
			refInOut = iterFindKey->second;
			return true;
		}
	}

	char szRead[1024] = { 0 };
	DWORD ret = GetPrivateProfileStringA(strAppName.c_str(), strKey.c_str(), refInOut.c_str(), szRead, sizeof(szRead), "本地机器专用.ini");
	if (ret == 0)
	{
        LOG(ERROR) << "GetPrivateProfileStringA:strAppName:" << strAppName << "strKey:" << strKey << ",szRead:" << szRead;
		return false;
	}

	LOG(INFO) << "GetPrivateProfileStringA:strAppName:" << strAppName << "strKey:" << strKey << ",szRead:" << szRead;
	s_map[strAppName][strKey] = szRead;
	refInOut = szRead;
	return true;
}

bool Try读Ini本地机器专用(uint32_t& refInOut, const std::string& strAppName, const std::string& strKey)
{
	std::string str(std::format("{0}", refInOut));
	if (!Try读Ini本地机器专用(str, strAppName, strKey))
		return false;

	refInOut = atoi(str.c_str());
	return true;
}
